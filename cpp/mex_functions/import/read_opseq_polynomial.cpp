/**
 * read_opseq_polynomial.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "read_opseq_polynomial.h"

#include "dictionary/raw_polynomial.h"

#include "scenarios/context.h"
#include "symbolic/symbol_table.h"
#include "symbolic/polynomial_factory.h"

#include "errors.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex {

    /** One input operator sequence with factor */
    class StagingMonomial {
    public:
        sequence_storage_t raw_sequence{};
        std::complex<double> factor = 1.0;
        std::optional<OperatorSequence> resolved_sequence = std::nullopt;
        symbol_name_t symbol_id = -1;
        bool conjugated = false;
        bool is_aliased = false;

    public:
        /** Turn raw sequence into contextualized operator sequence. */
        void raw_to_resolved(matlab::engine::MATLABEngine& engine,
                             const Context& context, const std::string& name);

        /** Try to find symbol, throw error if not found (should hold at least read lock). */
        void look_up_symbol(matlab::engine::MATLABEngine& engine,
                            const SymbolTable& symbols, const std::string& name);

        /** Try to find symbol, set to  '-1' if not found (should hold at least read lock). */
        bool look_up_symbol_or_fail_quietly(matlab::engine::MATLABEngine& engine,
                                            const SymbolTable& symbols);

        /** Try to find symbol, create it if not found (must hold write lock!). */
        void look_up_or_make_symbol(matlab::engine::MATLABEngine& engine,
                                    SymbolTable& symbols, const std::string& name);

    };

    void StagingMonomial::raw_to_resolved(matlab::engine::MATLABEngine& engine,
                         const Context& context, const std::string& name) {
        const size_t op_count = context.size();
        for (size_t seq_idx = 0; seq_idx < this->raw_sequence.size(); ++seq_idx) {
            const oper_name_t op = this->raw_sequence[seq_idx];
            if (op >= op_count) {
                std::stringstream errSS;
                errSS << "Operator '" << op << "' in " << name << ", position #" << (seq_idx+1) << " is out of range.";
                throw BadParameter{errSS.str()};
            }
        }
        this->resolved_sequence.emplace(std::move(this->raw_sequence), context);
    }

    void StagingMonomial::look_up_symbol(matlab::engine::MATLABEngine& engine,
                        const SymbolTable& symbols,  const std::string& name) {
        assert(this->resolved_sequence.has_value());
        const auto where = symbols.where(this->resolved_sequence.value());

        if (!where.found()) {
            std::stringstream errSS;
            errSS << "Sequence \"" << this->resolved_sequence.value().formatted_string() << "\""
                  <<  " in " << name
                  << " does not correspond to a known symbol, and automatic creation was disabled.";
            throw BadParameter{errSS.str()};
        }
        this->symbol_id = where->Id();
        this->is_aliased = where.is_aliased;
        this->conjugated = where.is_conjugated;
    }

    bool StagingMonomial::look_up_symbol_or_fail_quietly(matlab::engine::MATLABEngine& engine,
                                                         const SymbolTable& symbols) {
        assert(this->resolved_sequence.has_value());
        const auto where = symbols.where(this->resolved_sequence.value());

        if (!where.found()) {
            this->symbol_id = -1;
            this->conjugated = false;
            return false;
        }
        this->symbol_id = where->Id();
        this->is_aliased = where.is_aliased;
        this->conjugated = where.is_conjugated;
        return true;
    }

    void StagingMonomial::look_up_or_make_symbol(matlab::engine::MATLABEngine& engine,
                                            SymbolTable& symbols, const std::string& name) {
        assert(this->resolved_sequence.has_value());
        const auto where = symbols.where(this->resolved_sequence.value());
        if (where.found()) {
            this->symbol_id = where->Id();
            this->conjugated = where.is_conjugated;
            this->is_aliased = where.is_aliased;
        } else {
            this->symbol_id = symbols.merge_in(OperatorSequence{this->resolved_sequence.value()});
            this->conjugated = where.is_conjugated;
            this->is_aliased = where.is_aliased; // Even if not found, where can determine if moment is canonical.
        }
    }


    StagingPolynomial::StagingPolynomial(matlab::engine::MATLABEngine& engine,
                                         const matlab::data::Array& input,
                                         std::string input_name)
         : matlabEngine{engine}, name{std::move(input_name)} {


        // Check input is cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << name << " must be a cell array.";
            throw BadParameter{errSS.str()};
        }

        // Get size, and prepare stage
        const auto polynomial_cell = static_cast<const matlab::data::CellArray>(input);
        const size_t polynomial_size = polynomial_cell.getNumberOfElements();
        this->data = std::make_unique<StagingMonomial[]>(polynomial_size);
        this->data_length = polynomial_size;

        // Go through elements in cell
        for (size_t elem_index = 0; elem_index < polynomial_size; ++elem_index) {

            auto& output_monomial = this->data[elem_index];

            // Check symbol expression is cell
            if (polynomial_cell[elem_index].getType() != matlab::data::ArrayType::CELL) {
                std::stringstream errSS;
                errSS << name << " element #" << (elem_index+1) << " must be a cell array.";
                throw BadParameter{errSS.str()};
            }

            // Check symbol expression cell has 1 or 2 elements
            const matlab::data::CellArray symbol_expr_cell = polynomial_cell[elem_index];
            size_t symbol_expr_size = symbol_expr_cell.getNumberOfElements();
            if ((symbol_expr_size < 1) || (symbol_expr_size > 2)) {
                std::stringstream errSS;
                errSS << name << " element #" << (elem_index+1) << " must be a cell array "
                      << "containing an operator sequence and optionally a factor.";
                throw BadParameter{errSS.str()};
            }

            // Finally, attempt to read operators
            try {
                // Read op sequence, and translate
                std::vector<oper_name_t> raw_vec = read_as_vector<oper_name_t>(this->matlabEngine, symbol_expr_cell[0]);

                output_monomial.raw_sequence.reserve(raw_vec.size());
                size_t op_index = 0;
                for (auto op : raw_vec) {
                    if (op < 1) {
                        std::stringstream errSS;
                        errSS << "Operator '" << op << "' at position #"
                              << (op_index+1) << " is out of range.";
                        throw std::range_error{errSS.str()};
                    }
                    output_monomial.raw_sequence.emplace_back(op-1);
                    ++op_index;
                }

                // Read factor
                if (symbol_expr_size == 2) {
                    output_monomial.factor = read_as_complex_scalar<double>(this->matlabEngine, symbol_expr_cell[1]);
                } else {
                    output_monomial.factor = 1.0;
                }
            } catch (const std::exception& e) {
                std::stringstream errSS;
                errSS << "Error reading " << name << " element #" << (elem_index+1) << ": " << e.what();
                throw BadParameter{errSS.str()};
            }
        }
    }

    StagingPolynomial::~StagingPolynomial() noexcept = default;

    StagingPolynomial::StagingPolynomial(Moment::mex::StagingPolynomial&& rhs) noexcept = default;

    void StagingPolynomial::supply_context(const Context &context) {
        assert(this->data);
        for (size_t index = 0; index < this->data_length; ++index) {
            std::stringstream nameSS;
            nameSS << this->name << " element #" << (index + 1);
            this->data[index].raw_to_resolved(this->matlabEngine, context, nameSS.str());
        }
    }

    RawPolynomial StagingPolynomial::to_raw_polynomial() const {
        assert(this->data);
        RawPolynomial output;
        for (size_t index = 0; index < this->data_length; ++index) {
            if (!this->data[index].resolved_sequence.has_value()) [[unlikely]] {
                std::stringstream errSS;
                errSS << "RawPolynomial cannot be formed before sequences have been been resolved, but "
                      << this->name << " element #" << (index + 1) << " is missing.";
                throw BadParameter{errSS.str()};
            }
            output.emplace_back(this->data[index].resolved_sequence.value(), this->data[index].factor);
        }
        return output;
    }


    bool StagingPolynomial::find_symbols(const SymbolTable &symbols, bool fail_quietly) {
        assert(this->data);
        this->aliases_found = false;
        if (fail_quietly) {
            bool found_all_symbols = true;
            for (size_t index = 0; index < this->data_length; ++index) {
                this->data[index].look_up_symbol_or_fail_quietly(this->matlabEngine, symbols);
                if (this->data[index].symbol_id < 0) {
                    found_all_symbols = false;
                }
                if (this->data[index].is_aliased) {
                    this->aliases_found = true;
                }
            }
            symbols_resolved = found_all_symbols;
        } else {
            for (size_t index = 0; index < this->data_length; ++index) {
                std::stringstream nameSS;
                nameSS << this->name << " element #" << (index + 1);
                this->data[index].look_up_symbol(this->matlabEngine, symbols, nameSS.str());
                if (this->data[index].is_aliased) {
                    this->aliases_found = true;
                }
            }
            symbols_resolved = true;
        }
        return symbols_resolved;
    }

    void StagingPolynomial::find_or_register_symbols(SymbolTable &symbols) {
        assert(this->data);
        if (this->symbols_resolved) {
            return;
        }

        this->aliases_found = false;
        for (size_t index = 0; index < this->data_length; ++index) {
            if (this->data[index].symbol_id < 0) {
                std::stringstream nameSS;
                nameSS << this->name << " element #" << (index + 1);
                this->data[index].look_up_or_make_symbol(this->matlabEngine, symbols, nameSS.str());
                if (this->data[index].is_aliased) {
                    this->aliases_found = true;
                }
            }
        }
        this->symbols_resolved = true;
    }

    Polynomial StagingPolynomial::to_polynomial(const PolynomialFactory &factory) const {
        assert(this->symbols_resolved);
        Polynomial::storage_t resolved_symbols;
        resolved_symbols.reserve(this->data_length);
        for (size_t index = 0; index < this->data_length; ++index) {
            const auto& elem = this->data[index];
            resolved_symbols.emplace_back(elem.symbol_id, elem.factor, elem.conjugated);
        }

        return factory(std::move(resolved_symbols));
    }

}