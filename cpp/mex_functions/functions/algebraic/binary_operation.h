/**
 * binary_operation.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 * @see binary_operation_impl.h for template function definitions.
 *
 */

#pragma once


#include "../../mtk_function.h"

#include "integer_types.h"

#include "import/algebraic_operand.h"
#include "import/matrix_system_id.h"

#include "multithreading/maintains_mutex.h"

#include <cassert>

#include <concepts>
#include <iosfwd>
#include <stdexcept>
#include <span>
#include <utility>
#include <vector>

namespace Moment::mex::functions {

    enum class ProductType {
        /** Incompatible, or unknown, product. */
        Incompatible,
        /** Array / array, with unequal dimensions */
        MismatchedDimensions,
        /** Scalar / scalar. */
        OneToOne,
        /** Scalar / array. */
        OneToMany,
        /** Array / scalar. */
        ManyToOne,
        /** Array / array. */
        ManyToMany,
        /** Scalar / matrix. */
        OneToMatrix,
        /** Matrix / scalar. */
        MatrixToOne,
        /** Matrix / matrix. */
        MatrixToMatrix
    };

    /** Debug string representation of product type. */
    std::ostream& operator<<(std::ostream& os, ProductType pt);

    /** Determine how two operands might multiply. */
    ProductType determine_product_type(const AlgebraicOperand& lhs, const AlgebraicOperand& rhs) noexcept;

    /**
     * Parameters for a generic binary algebraic operation.
     */
    struct BinaryOperationParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** Left hand operand. */
        AlgebraicOperand lhs;

        /** Right hand operand. */
        AlgebraicOperand rhs;

    private:
        /** Resolved product type */
        ProductType resolved_product_type;

    public:
        explicit BinaryOperationParams(SortedInputs&& inputs);

        /** Resolved product type */
        [[nodiscard]] inline ProductType product_type() const noexcept {
            return this->resolved_product_type;
        }

        /** Debug synopsis of parameters. */
        [[nodiscard]] std::string to_string() const override;
    };


    class BinaryOperationException : public std::domain_error {
    public:
        explicit BinaryOperationException(std::string what) : std::domain_error{what} { }
    };


    template<std::derived_from<BinaryOperationParams> op_param_t, MTKEntryPointID entry_point_id>
    class BinaryOperation : public ParameterizedMTKFunction<op_param_t, entry_point_id> {
    public:
        using BinaryParams = op_param_t;

    protected:
        MatrixSystem * ms_ptr = nullptr;
        Context const * context_ptr = nullptr;
        PolynomialFactory const * pf_ptr = nullptr;

    public:
        BinaryOperation(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction<op_param_t, entry_point_id>{matlabEngine, storage} {
                this->min_inputs = 3;
                this->max_inputs = 3;
                this->min_outputs = 1;
                this->max_outputs = 4;
        }

        virtual ~BinaryOperation() noexcept = default;

        void operator()(IOArgumentRange output, BinaryParams &input) final;

    private:
        void polynomial_by_polynomial(IOArgumentRange output, BinaryParams& input);

        void polynomial_by_matrix(IOArgumentRange output, BinaryParams& input);

        void matrix_by_matrix(IOArgumentRange output, BinaryParams& input);

    protected:

        /**
         * Additional set-up performed after acquiring matrix system, before operands are resolved.
         */
        virtual void additional_setup(BinaryParams& input) { }

        /**
         * Scalar LHS by matrix RHS -> matrix output.
         * Must be overloaded, to implement operand behaviour!
         */
        virtual std::pair<ptrdiff_t, const Moment::SymbolicMatrix&>
        one_to_matrix(const MaintainsMutex::WriteLock& write_lock,
                      const RawPolynomial& lhs, const SymbolicMatrix& rhs) {
            throw BinaryOperationException{"Polynomial x Matrix not implemented."};
        }

        /**
         * Scalar LHS by matrix RHS -> matrix output.
         * Must be overloaded, to implement operand behaviour.
         */
        virtual std::pair<ptrdiff_t, const Moment::SymbolicMatrix&>
        matrix_to_one(const MaintainsMutex::WriteLock& write_lock,
                      const SymbolicMatrix& lhs, const RawPolynomial& rhs) {
            throw BinaryOperationException{"Matrix x Polynomial not implemented."};
        }

        /**
         * Matrix LHS by matrix RHS -> matrix output.
         * Must be overloaded, to implement operand behaviour.
         */
        virtual std::pair<ptrdiff_t, const Moment::SymbolicMatrix&>
        matrix_to_matrix(const MaintainsMutex::WriteLock& write_lock,
                         const SymbolicMatrix& lhs, const SymbolicMatrix& rhs) {
            throw BinaryOperationException{"Matrix x Matrix not implemented."};
        }

        /**
         * Scalar LHS by scalar RHS -> scalar output.
         * Must be overloaded, to implement operand behaviour.
         */
        virtual RawPolynomial one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) {
            throw BinaryOperationException{"Polynomial x Polynomial not implemented."};
        }

        /**
         * Scalar LHS by array RHS -> array output.
         * Default implementation is repeated 1-to-1 behaviour.
         */
        virtual std::vector<RawPolynomial> one_to_many(const RawPolynomial& lhs,
                                                       std::span<const RawPolynomial> rhs_list) {
            std::vector<RawPolynomial> output;
            output.reserve(rhs_list.size());
            for (const auto& rhs : rhs_list) {
                output.emplace_back(this->one_to_one(lhs, rhs));
            }
            return output;
        }

        /**
         * Array LHS by scalar RHS -> array output.
         * Default implementation is repeated 1-to-1 behaviour.
         */
        virtual std::vector<RawPolynomial> many_to_one(std::span<const RawPolynomial> lhs_list,
                                                       const RawPolynomial& rhs) {
            std::vector<RawPolynomial> output;
            output.reserve(lhs_list.size());
            for (const auto& lhs : lhs_list) {
                output.emplace_back(this->one_to_one(lhs, rhs));
            }
            return output;
        }

        /**
         * Array RHS by array RHS -> array output.
         * Default implementation is repeated 1-to-1 behaviour.
         */
        virtual std::vector<RawPolynomial> many_to_many(std::span<const RawPolynomial> lhs_list,
                                                        std::span<const RawPolynomial> rhs_list) {
            assert(lhs_list.size() == rhs_list.size());
            std::vector<RawPolynomial> output;
            output.reserve(lhs_list.size());
            for (size_t index = 0, count = lhs_list.size(); index < count; ++index) {
                output.emplace_back(this->one_to_one(lhs_list[index], rhs_list[index]));
            }
            return output;
        }

    };



}