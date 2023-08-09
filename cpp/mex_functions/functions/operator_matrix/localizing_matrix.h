/**
 * localizing_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"

#include "import/read_polynomial.h"

#include "matrix_system/localizing_matrix_index.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"

#include <variant>

namespace Moment {
    class Context;
    namespace mex {
        class StagingPolynomial;
    }
}

namespace Moment::mex::functions  {

    struct LocalizingMatrixParams : public OperatorMatrixParams {
    public:
        using raw_word_storage_t = std::variant<std::vector<oper_name_t>,
                                                std::vector<raw_sc_data>,
                                                std::unique_ptr<StagingPolynomial>>;


        /** Matrix level. */
        unsigned long hierarchy_level = 0;

    protected:
        /** Matrix word. */
        raw_word_storage_t localizing_expression;

    public:
        /** Add one to all operator IDs. */
        bool matlab_indexing = true;


        enum class ExpressionType {
            /** Unknown */
            Unknown,
            /** Monomial, defined by operator sequence. */
            OperatorSequence,
            /** Polynomial, defined by symbol cell. */
            SymbolCell,
            /** Polynomial, defined by operators. */
            OperatorCell
        } expression_type = ExpressionType::OperatorSequence;

    public:

        /** Monomial matrix word as operator sequence. */
        [[nodiscard]] std::vector<oper_name_t>& localizing_word() {
            return std::get<0>(this->localizing_expression);
        }

       /** Monomial matrix word as operator sequence. */
        [[nodiscard]] const std::vector<oper_name_t>& localizing_word() const {
            return std::get<0>(this->localizing_expression);
        }

        /** Raw input for symbol cell input. */
        [[nodiscard]] std::vector<raw_sc_data>& localizing_symbol_cell() {
            return std::get<1>(this->localizing_expression);
        }

        /** Raw input for symbol cell input. */
        [[nodiscard]] const std::vector<raw_sc_data>& localizing_symbol_cell() const {
            return std::get<1>(this->localizing_expression);
        }

        /** Staging polynomial for operator cell input. */
        [[nodiscard]] std::unique_ptr<StagingPolynomial>& localizing_operator_cell() {
            return std::get<2>(this->localizing_expression);
        }

        /** Staging polynomial for operator cell input. */
        [[nodiscard]] const StagingPolynomial* localizing_operator_cell() const {
            return std::get<2>(this->localizing_expression).get();
        }

        explicit LocalizingMatrixParams(SortedInputs&& inputs);

        ~LocalizingMatrixParams() noexcept;
        /**
         * Use the supplied context to create an index for the requested localizing matrix.
         */
        [[nodiscard]] LocalizingMatrixIndex to_monomial_index(const Context& context) const;

        [[nodiscard]] PolynomialLMIndex to_polynomial_index(const PolynomialFactory& factory) const;

        [[nodiscard]] PolynomialLMIndex to_polynomial_index(const PolynomialFactory& factory,
                                                            const Context& context);

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level, word]"; }

    private:
        void read_localizing_expression(const matlab::data::Array& expr);

    };

    class LocalizingMatrix
        : public Moment::mex::functions::OperatorMatrix<LocalizingMatrixParams, MTKEntryPointID::LocalizingMatrix> {
    public:
        explicit LocalizingMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}
