/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "storage_manager.h"

#include "import/read_opseq_polynomial.h"

#include "matrix/operator_matrix/localizing_matrix.h"


#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"
#include "matrix/polynomial_matrix.h"

#include <memory>


namespace Moment::mex::functions {

    LocalizingMatrixParams::LocalizingMatrixParams(SortedInputs &&inputs) : OperatorMatrixParams(std::move(inputs)) { }

    LocalizingMatrixParams::~LocalizingMatrixParams() noexcept = default;


    void LocalizingMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent.

        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->matlab_indexing = true;
        } else if (this->flags.contains(u"zero_indexing")) {
            this->matlab_indexing = false;
        }

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get localizing word sequence
        auto& word_param = this->find_or_throw(u"word");
        this->read_localizing_expression(word_param);
    }

    void LocalizingMatrixParams::extra_parse_inputs() {
        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->matlab_indexing = true;
        } else if (this->flags.contains(u"zero_indexing")) {
            this->matlab_indexing = false;
        }

        // No named parameters... try to interpret inputs as matrix system, depth and word.
        assert(this->inputs.size() == 3); // should be guaranteed by parent.
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);

        this->read_localizing_expression(inputs[2]);

    }

    void LocalizingMatrixParams::read_localizing_expression(const matlab::data::Array &expr) {

        // Get input type flag
        if (this->flags.contains(u"symbols")) {
            this->expression_type = ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            this->expression_type = ExpressionType::OperatorCell;
        }

        // Handle input based on type:
        switch (expr.getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                // Check type is consistent
                if (ExpressionType::Unknown == this->expression_type) {
                    this->expression_type = ExpressionType::OperatorSequence;
                } else if (ExpressionType::OperatorSequence != this->expression_type) {
                    throw_error(matlabEngine, errors::bad_param, "Cell input specified, but operator sequence supplied.");
                }
                // Read operator sequence
                this->localizing_expression.emplace<0>(read_integer_array<oper_name_t>(matlabEngine,
                                                                                       "Localizing expression", expr));
                break;
            case matlab::data::ArrayType::CELL:
                // Check type is consistent
                if (ExpressionType::Unknown == this->expression_type) {
                    this->expression_type = ExpressionType::OperatorCell;
                } else if (ExpressionType::OperatorSequence == this->expression_type) {
                    throw_error(matlabEngine, errors::bad_param, "Operator sequence specified, but cell array supppliud.");
                }
                if (ExpressionType::SymbolCell == this->expression_type) {
                    this->localizing_expression.emplace<1>(
                            read_raw_polynomial_data(this->matlabEngine, "Localizing expression", expr));
                } else {
                    this->localizing_expression.emplace<2>(
                            std::make_unique<StagingPolynomial>(this->matlabEngine, expr, "Localizing expression"));
                }
                break;
            default:
            case matlab::data::ArrayType::UNKNOWN:
                throw_error(this->matlabEngine, errors::bad_param,
                            "Localizing expression must be an operator sequence, or a polynomial cell definition.");
        }
    }

    LocalizingMatrixIndex LocalizingMatrixParams::to_monomial_index(const Context& context) const {
        // Check mode
        if (debug_mode && (ExpressionType::OperatorSequence != this->expression_type)) {
            throw_error(matlabEngine, errors::internal_error, "No monomial defined by this LocalizingMatrixIndex.");
        }

        // Do we have to offset?
        sequence_storage_t oper_copy{};
        oper_copy.reserve(this->localizing_word().size());
        for (auto o : this->localizing_word()) {
            if (this->matlab_indexing) {
                if (0 == o) {
                    throw_error(matlabEngine, errors::bad_param,
                                "Operator with index 0 in localizing word is out of range.");
                }
                o -= 1;
            }

            // Check in range
            if ((o < 0) || (o >= context.size())) {
                std::stringstream errSS;
                errSS << "Operator " << (this->matlab_indexing ? o + 1 : o) << " at index ";
                errSS << (oper_copy.size() + 1);
                errSS << " is out of range.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }
            oper_copy.emplace_back(o);
        }

        // Copy and construct LMI
        return LocalizingMatrixIndex{this->hierarchy_level, OperatorSequence{std::move(oper_copy), context}};
    }

    PolynomialLMIndex LocalizingMatrixParams::to_polynomial_index(const PolynomialFactory& factory) const {
        switch (this->expression_type) {
            case ExpressionType::SymbolCell:
                return PolynomialLMIndex{this->hierarchy_level, raw_data_to_polynomial(this->matlabEngine, factory,
                                                                                       this->localizing_symbol_cell())};
            case ExpressionType::OperatorCell: {
                const auto& poly = *this->localizing_operator_cell();
                if (!poly.ready()) {
                    throw_error(matlabEngine, errors::internal_error,
                                "OperatorCell polynomial has not yet been resolved into symbols.");
                }
                return PolynomialLMIndex{this->hierarchy_level, poly.to_polynomial(factory)};
            }
            default:
                throw_error(matlabEngine, errors::internal_error,
                            "Localizing expression was not given as symbol cell array.");
        }

    }

    bool LocalizingMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    LocalizingMatrix::LocalizingMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, word] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"operators");
        this->mutex_params.add_mutex(u"symbols", u"operators");

        this->flag_names.emplace(u"zero_indexing");
        this->flag_names.emplace(u"matlab_indexing");
        this->mutex_params.add_mutex(u"zero_indexing", u"matlab_indexing");

        this->max_inputs = 3;
    }

    namespace {
        std::pair<size_t, const Moment::SymbolicMatrix &> getMonoLM(MatrixSystem &system, LocalizingMatrixParams& input) {
            // Try to get via read-lock only
            auto read_lock = system.get_read_lock();
            auto lmi = input.to_monomial_index(system.Context());
            auto offset = system.LocalizingMatrix.find_index(lmi);
            if (offset >= 0) {
                return {offset, system[offset]};
            }

            // Try to create
            read_lock.unlock();
            return system.LocalizingMatrix.create(lmi);
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolySymbolLM(MatrixSystem &system, LocalizingMatrixParams& input) {
            // Try to get in read-only mode
            auto read_lock = system.get_read_lock();
            auto plmi = input.to_polynomial_index(system.polynomial_factory());
            auto offset = system.PolynomialLocalizingMatrix.find_index(plmi);
            if (offset >= 0) {
                return {offset, system[offset]};
            }

            // Try to create polynomial matrix
            read_lock.unlock();
            return system.PolynomialLocalizingMatrix.create(plmi);
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolyOpLM(MatrixSystem &system, LocalizingMatrixParams& input) {
            // Can expression be parsed without new symbols?
            auto read_lock = system.get_read_lock();
            auto& context = system.Context();
            const auto& factory = system.polynomial_factory();
            auto& symbol_table = system.Symbols();

            auto& staging_poly = *input.localizing_operator_cell();
            staging_poly.supply_context(context);
            const bool found_all = staging_poly.find_symbols( symbol_table, true);

            // Not all symbols found, so switch to write lock
            if (!found_all) {
                read_lock.unlock(); // may have to wait...!

                auto write_lock = system.get_write_lock();
                staging_poly.find_or_register_symbols(symbol_table);
                auto index = input.to_polynomial_index(factory);
                return system.PolynomialLocalizingMatrix.create(write_lock, index);
            }

            // Try to read LM entirely in read only mode
            auto index = input.to_polynomial_index(factory);
            auto offset = system.PolynomialLocalizingMatrix.find_index(index);
            if (offset >= 0) {
                return {offset, system[offset]};
            }

            // Otherwise, create (implicitly acquiring write lock)
            read_lock.unlock();
            return system.PolynomialLocalizingMatrix.create(index);
        }
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    LocalizingMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        auto &input = dynamic_cast<LocalizingMatrixParams&>(inputOMP);

        // Switch based on type
        try {
            switch (input.expression_type) {
                case LocalizingMatrixParams::ExpressionType::OperatorSequence:
                    return getMonoLM(system, input);
                case LocalizingMatrixParams::ExpressionType::SymbolCell:
                    return getPolySymbolLM(system, input);
                case LocalizingMatrixParams::ExpressionType::OperatorCell:
                    return getPolyOpLM(system, input);
                default:
                case LocalizingMatrixParams::ExpressionType::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown localizing expression type.");
            }
        } catch (std::exception& e) {
            throw_error(matlabEngine, errors::internal_error,
                        std::string("A problem occurred while retrieving/generating localizing matrix: ") + e.what());
        }

    }
}