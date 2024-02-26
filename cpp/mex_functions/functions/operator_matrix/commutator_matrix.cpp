/**
 * commutator_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "commutator_matrix.h"

#include "errors.h"
#include "storage_manager.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "import/read_localizing_matrix_indices.h"
#include "import/read_opseq_polynomial.h"
#include "import/read_polynomial.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    void CommutatorMatrixParams::extra_parse_params()  {
        assert(inputs.empty()); // Should be guaranteed by parent

        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);


        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get word
        auto& word_param = this->find_or_throw(u"word");
        this->lmi_importer_ptr->read_localizing_expression(word_param, expr_type);

        // Extra params
        this->parse_optional_params();
    }

    void CommutatorMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as [ref_id, level, word]
        assert(this->inputs.size() == 3); // should be guaranteed by parent

        // Do we offset word read by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // Read NPA level
        this->lmi_importer_ptr->read_level(inputs[1]);

        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get word
        this->lmi_importer_ptr->read_localizing_expression(inputs[2], expr_type);

        // Extra params
        this->parse_optional_params();
    }

    void CommutatorMatrixParams::parse_optional_params() {
        // Get nearest-neighbour restriction if any
        this->find_and_parse(u"neighbours", [this](const matlab::data::Array& nn_param) {
            this->lmi_importer_ptr->read_nearest_neighbour(nn_param);
        });

        // Get matrix type
        if (this->flags.contains(u"anticommutator")) {
            this->requested_matrix = RequestedMatrix::Anticommutator;
        } else if (this->flags.contains(u"commutator")) {
            this->requested_matrix = RequestedMatrix::Commutator;
        }
    }

    [[nodiscard]] bool CommutatorMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    CommutatorMatrixParams::CommutatorMatrixParams(SortedInputs&& input)
        : OperatorMatrixParams(std::move(input)) {
        this->lmi_importer_ptr = std::make_unique<LocalizingMatrixIndexImporter>(this->matlabEngine);

    }

    CommutatorMatrixParams::~CommutatorMatrixParams() noexcept = default;



    CommutatorMatrix::CommutatorMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, extensions] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        // Accept NN parameter
        this->param_names.emplace(u"neighbours");

        // Word type?
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"operators");
        this->mutex_params.add_mutex(u"symbols", u"operators");

        // Matrix type?
        this->flag_names.emplace(u"anticommutator");
        this->flag_names.emplace(u"commutator");
        this->mutex_params.add_mutex(u"anticommutator", u"commutator");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    namespace {
        template<bool anticommutator>
        std::pair<size_t, const Moment::SymbolicMatrix&>
        getMonoCM(matlab::engine::MATLABEngine& matlabEngine, Pauli::PauliMatrixSystem& pauli_system,
                  CommutatorMatrixParams& input, Multithreading::MultiThreadPolicy mt_policy) {
            using Index = std::conditional_t<anticommutator, Pauli::AnticommutatorMatrixIndex, Pauli::CommutatorMatrixIndex>;

            auto read_lock = pauli_system.get_read_lock();
            auto plmi = static_cast<Index>(input.lmi_importer().to_pauli_monomial_index());

            const auto offset = [&pauli_system, &plmi]() -> ptrdiff_t {
                if constexpr (anticommutator) {
                    return pauli_system.AnticommutatorMatrices.find_index(plmi);
                } else {
                    return pauli_system.CommutatorMatrices.find_index(plmi);
                }
            }();
            //auto offset = pauli_system.CommutatorMatrices.find_index(plmi);

            if (offset >= 0) {
                return {offset, pauli_system[offset]};
            }

            // Try to create
            read_lock.unlock();
            if constexpr (anticommutator) {
                return pauli_system.AnticommutatorMatrices.create(plmi, mt_policy);
            } else {
                return pauli_system.CommutatorMatrices.create(plmi, mt_policy);
            }
        }

        template<bool anticommutator>
        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolyCMExistingSymbols(matlab::engine::MATLABEngine& matlabEngine,
                                 MaintainsMutex::ReadLock&& read_lock,
                                 Pauli::PauliMatrixSystem &system, CommutatorMatrixParams& input,
                                 Multithreading::MultiThreadPolicy mt_policy) {
            using Index = std::conditional_t<anticommutator, Pauli::PolynomialAnticommutatorMatrixIndex,
                                                             Pauli::PolynomialCommutatorMatrixIndex>;

            assert(system.is_locked_read_lock(read_lock));

            auto plmi = static_cast<Index>(input.lmi_importer().to_pauli_polynomial_index());

            // Try to get in read-only mode
            const auto offset = [&system, &plmi]() -> ptrdiff_t {
                if constexpr (anticommutator) {
                    return system.PolynomialAnticommutatorMatrices.find_index(plmi);
                } else {
                    return system.PolynomialCommutatorMatrices.find_index(plmi);
                }
            }();

            if (offset >= 0) {
                return {offset, system[offset]}; // ~read_lock
            }

            // Try to create polynomial matrix
            read_lock.unlock();
            if constexpr (anticommutator) {
                return system.PolynomialAnticommutatorMatrices.create(std::move(plmi), mt_policy);
            } else {
                return system.PolynomialCommutatorMatrices.create(std::move(plmi), mt_policy);
            }

        }

        template<bool anticommutator>
        inline std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolySymbolCM(matlab::engine::MATLABEngine& matlabEngine, Pauli::PauliMatrixSystem &system,
                        CommutatorMatrixParams& input,
                        Multithreading::MultiThreadPolicy mt_policy) {
            return getPolyCMExistingSymbols<anticommutator>(matlabEngine, system.get_read_lock(),
                                                            system, input, mt_policy);
        }

        template<bool anticommutator>
        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolyOpCM(matlab::engine::MATLABEngine& matlabEngine,
                    Pauli::PauliMatrixSystem &system, CommutatorMatrixParams& input,
                    Multithreading::MultiThreadPolicy mt_policy) {

            using Index = std::conditional_t<anticommutator, Pauli::PolynomialAnticommutatorMatrixIndex,
                                                             Pauli::PolynomialCommutatorMatrixIndex>;

            // Can expression be parsed without new symbols?
            auto symbol_read_lock = system.get_read_lock();
            const bool found_all = input.lmi_importer().attempt_to_find_symbols_from_op_cell(symbol_read_lock);

            // Not all symbols found, so switch to write lock
            if (!found_all) {
                symbol_read_lock.unlock();
                auto write_lock = system.get_write_lock();
                input.lmi_importer().register_symbols_in_op_cell(write_lock);
                auto index = static_cast<Index>(input.lmi_importer().to_pauli_polynomial_index());

                if constexpr (anticommutator) {
                    // Create or retrieve
                    return system.PolynomialAnticommutatorMatrices.create(write_lock, std::move(index), mt_policy);
                } else {
                    // Create or retrieve
                    return system.PolynomialCommutatorMatrices.create(write_lock, std::move(index), mt_policy);
                }
            }

            // Fall-back to normal retrieval
            return getPolyCMExistingSymbols<anticommutator>(matlabEngine, std::move(symbol_read_lock),
                                                            system, input, mt_policy);
        }

        template<bool anticommutator>
        std::pair<size_t, const Moment::SymbolicMatrix &>
        getAliasedPolyOpCM(matlab::engine::MATLABEngine& matlabEngine,
                    Pauli::PauliMatrixSystem &system, CommutatorMatrixParams& input,
                    Multithreading::MultiThreadPolicy mt_policy) {
            // Can expression be parsed without new symbols?
            auto symbol_read_lock = system.get_read_lock();
            auto& lmi_importer = input.lmi_importer();
            lmi_importer.supply_context_only(symbol_read_lock);
            auto [raw_level, raw_poly] = input.lmi_importer().to_pauli_raw_polynomial_index();
            symbol_read_lock.unlock();

            // System will call its own write locks:~
            if constexpr (anticommutator) {
                return system.create_and_register_anticommutator_matrix(raw_level, raw_poly, mt_policy);
            } else {
                return system.create_and_register_commutator_matrix(raw_level, raw_poly, mt_policy);
            }

        }
    }

    std::pair<size_t, const Moment::SymbolicMatrix&>
    CommutatorMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp)  {
        // Get extended parameters
        auto& cmp = dynamic_cast<CommutatorMatrixParams&>(omp);

        // Get MT policy
        const auto mt_policy = this->settings->get_mt_policy();

        // Get Pauli matrix system
        auto * pauli_ptr = dynamic_cast<Pauli::PauliMatrixSystem*>(&system);
        if (nullptr == pauli_ptr) {
            throw BadParameter{"Matrix system reference was not a Pauli scenario."};
        }
        auto& pauli_system = *pauli_ptr;

        // Attach matrix system to index reader
        cmp.lmi_importer().link_matrix_system(&pauli_system);

        const bool anticommutator = cmp.requested_matrix == CommutatorMatrixParams::RequestedMatrix::Anticommutator;

        // Switch based on type
        try {
            switch (cmp.lmi_importer().get_expression_type()) {
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence:
                    if (anticommutator) {
                        return getMonoCM<true>(matlabEngine, pauli_system, cmp, mt_policy);
                    } else {
                        return getMonoCM<false>(matlabEngine, pauli_system, cmp, mt_policy);
                    }
                case LocalizingMatrixIndexImporter::ExpressionType::SymbolCell:
                    if (!this->quiet && pauli_system.pauliContext.can_have_aliases()) {
                        print_warning(matlabEngine,
                          "If symmetrization is enabled, symbol cell input might produce unexpected results:\n"
                          "The input Polynomial will be symmetrized before its (anti)commutators are calculated!");
                    }

                    if (anticommutator) {
                        return getPolySymbolCM<true>(matlabEngine, pauli_system, cmp, mt_policy);
                    } else {
                        return getPolySymbolCM<false>(matlabEngine, pauli_system, cmp, mt_policy);
                    }
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorCell:
                    if (pauli_system.pauliContext.can_have_aliases()) {
                        if (anticommutator) {
                            return getAliasedPolyOpCM<true>(matlabEngine, pauli_system, cmp, mt_policy);
                        } else {
                            return getAliasedPolyOpCM<false>(matlabEngine, pauli_system, cmp, mt_policy);
                        }
                    } else {
                        if (anticommutator) {
                            return getPolyOpCM<true>(matlabEngine, pauli_system, cmp, mt_policy);
                        } else {
                            return getPolyOpCM<false>(matlabEngine, pauli_system, cmp, mt_policy);
                        }
                    }
                default:
                case LocalizingMatrixIndexImporter::ExpressionType::Unknown:
                    throw InternalError{"Unknown localizing expression type."};
            }
        } catch (std::exception& e) {
            throw InternalError{std::string("A problem occurred while retrieving/generating localizing matrix: ")
                                + e.what()};
        }
    }

}