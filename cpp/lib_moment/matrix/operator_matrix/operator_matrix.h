/**
 * operator_matrix.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/monomial_matrix.h"

#include "scenarios/context.h"

#include "dictionary/operator_sequence.h"
#include "dictionary/osg_pair.h"

#include "multithreading/multithreading.h"

#include "symbolic/monomial.h"
#include "symbolic/symbol_table.h"

#include "tensor/square_matrix.h"

#include "is_hermitian.h"

#include <cassert>

#include <map>
#include <memory>
#include <optional>
#include <span>

namespace Moment {

    class Polynomial;
    class SymbolTable;
    class SymbolMatrix;

    class OperatorMatrix : public SquareMatrix<OperatorSequence> {
    public:
        const Context& context;
    private:
        bool hermitian = false;
        std::optional<NonHInfo> non_hermitian_elem;

    public:
        explicit OperatorMatrix(const Context& context, size_t dimension, std::vector<OperatorSequence>&& op_seq_data);

        OperatorMatrix(OperatorMatrix&& rhs) noexcept = default;

        virtual ~OperatorMatrix() noexcept;

        [[nodiscard]] inline size_t Dimension() const noexcept { return this->dimension; }

        /**
         * True if the matrix is Hermitian.
         */
        [[nodiscard]] inline bool is_hermitian() const noexcept { return this->hermitian; }

        /**
         * Get first row and column indices of non-hermitian element in matrix, if any. Otherwise nullopt.
         */
        [[nodiscard]] inline std::optional<Index> nonhermitian_index() const noexcept {
            if (this->non_hermitian_elem.has_value()) {
                return this->non_hermitian_elem->Index;
            } else {
                return std::nullopt;
            }
        }

        /**
         * Operator matrices usually are made in an algorithmic manner, and can provide a name to their symbolization.
         */
        [[nodiscard]] virtual std::string description() const {
            return "Operator Matrix";
        }

        /**
         * Provide access to matrix generators
         */
         [[nodiscard]] virtual const OSGPair& generators() const;

        /** Apply the properties from this operator matrix to the supplied matrix. */
        void set_properties(SymbolicMatrix& matrix) const;

        /** Create a new operator matrix by pre-multiplying by an operator sequence */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        pre_multiply(const OperatorSequence& lhs,
                    Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /** Create a new operator matrix by post-multiplying by an operator sequence */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        post_multiply(const OperatorSequence& rhs,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of raw polynomial, by pre-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        pre_multiply(const RawPolynomial& lhs,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of raw polynomial, by post-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        post_multiply(const RawPolynomial& rhs,
                      Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of polynomial, by pre-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        pre_multiply(const Polynomial& lhs, const SymbolTable& symbols,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of polynomial, by post-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        post_multiply(const Polynomial& rhs, const SymbolTable& symbols,
                      Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix by identifying moment aliases.
         * @param mt_policy Whether the simplification should be done with multithreading (Never, Optional, Always).
         * @return Newly created OperatorMatrix, or nullptr if context does not admit aliases.
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        simplify_as_moments(Multithreading::MultiThreadPolicy policy
            = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Creates a copy of this matrix
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        clone(Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        friend class SymbolicMatrix;
        friend class MonomialMatrix;
        friend class PolynomialMatrix;
    };
}