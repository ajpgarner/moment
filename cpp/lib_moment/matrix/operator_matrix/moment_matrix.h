/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"

#include "multithreading/multithreading.h"

#include <optional>
#include <stdexcept>

namespace Moment {

    namespace errors {
        class hermitian_failure : public std::logic_error {
        public:
            explicit hermitian_failure(const std::string& what) : std::logic_error{what} { }
        };
    }

    class OperatorSequenceGenerator;

    /**
     * MomentMatrix, of operators.
     */
    class MomentMatrix : public OperatorMatrix {
    public:
        /** The Level of moment matrix defined */
        const size_t hierarchy_level;

    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param level The hierarchy depth.
         * @param mt_policy Whether or not to use multi-threaded creation.
         */
        MomentMatrix(const Context& context, size_t level,
                     std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

        MomentMatrix(const MomentMatrix&) = delete;

        MomentMatrix(MomentMatrix&& src) noexcept;

        ~MomentMatrix() noexcept;

        /**
         * The hierarchy depth of this moment matrix.
         */
        [[nodiscard]] constexpr size_t Level() const noexcept { return this->hierarchy_level; }

        /**
         * The generators associated with this matrix
         */
         [[nodiscard]] const OperatorSequenceGenerator& Generators() const;

         [[nodiscard]] std::string description() const override;

    public:
        /**
         * If supplied input is symbol matrix associated with a monomial moment matrix, extract that moment matrix.
         * Otherwise, returns std::nullopt.
         */
        static const MomentMatrix* as_monomial_moment_matrix_ptr(const SymbolicMatrix& input) noexcept;

        /**
         * Full creation stack, with possible multithreading.
         */
        static std::unique_ptr<MonomialMatrix>
        create_matrix(const Context& context, SymbolTable& symbols, size_t level,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);


        friend class MomentMatrixCreationContext;
    };
}