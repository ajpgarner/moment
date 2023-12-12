/**
 * commutator_matrix.h
 *
 * Defines methods for generating and instantiating matrices of the form [MM, x] and {MM, x} for monomial x and moment
 * matrix MM.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "pauli_localizing_matrix_indices.h"

#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/operator_matrix/operator_matrix_impl.h"

#include "matrix_system/matrix_indices.h"

#include "pauli_context.h"
#include "pauli_dictionary.h"

namespace Moment::Pauli {
    class PauliMatrixSystem;

    using CommutatorMatrixIndex = PauliLocalizingMatrixIndex;

    /**
     * Generate 'Pauli' (anti-)commutator matrix, possibly limited to nearest-neighbours in top row.
     */
    template<bool anti_commutator = false>
    struct CommutatorMatrixGenerator {
    public:
        using Index = CommutatorMatrixIndex;
        using OSGIndex = NearestNeighbourIndex;

        const Index index;
        const PauliContext& context;

        CommutatorMatrixGenerator(const PauliContext& context, Index index_in)
                : context{context}, index{std::move(index_in)} { }

        [[nodiscard]] inline OperatorSequence
        operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
            if constexpr(anti_commutator) {
                return context.anticommutator(lhs * rhs, index.Word);
            } else {
                return context.commutator(lhs * rhs, index.Word);
            }
        }

        /** Pauli (anti-)commutator matrices could be Hermitian if:
         *      Commutator: word is imaginary (if word is real, then anti-hermitian).
         *  Anticommutator: word is real (if word is imaginary, then anti-hermitian).
         */
        [[nodiscard]] inline constexpr static bool
        should_be_hermitian(const Index& index) noexcept {
            if constexpr (anti_commutator) {
                return !is_imaginary(index.Word.get_sign());
            }
            return is_imaginary(index.Word.get_sign());
        }

        /** Pauli (anti-)commutator matrices have a prefactor of 2 */
        [[nodiscard]] inline constexpr static std::complex<double>
        determine_prefactor(const Index& /**/) noexcept { return std::complex<double>{2.0, 0.0}; }

        /** Pass-through index to get OSG index. */
        [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const Index& input) {
            return input.Index;
        }

        /** Get nearest-neighbour OSGs */
        [[nodiscard]] inline static const OSGPair& get_generators(const PauliContext& context,
                                                                  const OSGIndex& index) {
            return context.pauli_dictionary().NearestNeighbour(index);
        }

    };

    static_assert(generates_operator_matrices<CommutatorMatrixGenerator<false>, CommutatorMatrixIndex, PauliContext>);
    static_assert(generates_operator_matrices<CommutatorMatrixGenerator<true>, CommutatorMatrixIndex, PauliContext>);


    /**
     * Monomial commutator matrix
     */
    class MonomialCommutatorMatrix;
    class MonomialCommutatorMatrix
            : public OperatorMatrixImpl<CommutatorMatrixIndex, PauliContext, CommutatorMatrixGenerator<false>,
                                        MonomialCommutatorMatrix> {
    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param level The hierarchy depth.
         * @param mt_policy Whether or not to use multi-threaded creation.
         */
        MonomialCommutatorMatrix(const PauliContext& context, const PauliLocalizingMatrixIndex& plmi,
                              std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
              :  OperatorMatrixImpl<CommutatorMatrixIndex, PauliContext, CommutatorMatrixGenerator<false>,
                                    MonomialCommutatorMatrix>{context, plmi, std::move(op_seq_mat)} { }
        /**
         * Description of the commutator matrix.
         */
        [[nodiscard]] std::string description() const override;

    };

    /**
     * Monomial anti-commutator matrix
     */
    class MonomialAnticommutatorMatrix;
    class MonomialAnticommutatorMatrix
            : public OperatorMatrixImpl<CommutatorMatrixIndex, PauliContext, CommutatorMatrixGenerator<true>,
                                        MonomialAnticommutatorMatrix> {
    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param level The hierarchy depth.
         * @param mt_policy Whether or not to use multi-threaded creation.
         */
        MonomialAnticommutatorMatrix(const PauliContext& context, const PauliLocalizingMatrixIndex& plmi,
                              std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
              :  OperatorMatrixImpl<CommutatorMatrixIndex, PauliContext, CommutatorMatrixGenerator<true>,
                                    MonomialAnticommutatorMatrix>{context, plmi, std::move(op_seq_mat)} { }
        /**
         * Description of the anti-commutator matrix.
         */
        [[nodiscard]] std::string description() const override;

    };

    /**
     * Factory to make commutator matrices (possibly restricted to nearest neighbours)
     */
    class MonomialCommutatorMatrixFactory {
    public:
        using Index = CommutatorMatrixIndex;

    private:
        PauliMatrixSystem& system;

    public:
        explicit MonomialCommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

        explicit MonomialCommutatorMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index,
                   Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, MonomialMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& pmi) const;
    };

    static_assert(makes_matrices<MonomialCommutatorMatrixFactory, MonomialMatrix, CommutatorMatrixIndex>);
    using CommutatorMatrixIndices = MappedMatrixIndices<MonomialMatrix, CommutatorMatrixIndex,
                                                        MonomialCommutatorMatrixFactory, PauliMatrixSystem>;

    /**
     * Factory to make anti-commutator matrices (possibly restricted to nearest neighbours)
     */
    class MonomialAnticommutatorMatrixFactory {
    public:
        using Index = CommutatorMatrixIndex;

    private:
        PauliMatrixSystem& system;

    public:
        explicit MonomialAnticommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

        explicit MonomialAnticommutatorMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index,
                   Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, MonomialMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& pmi) const;
    };


    static_assert(makes_matrices<MonomialAnticommutatorMatrixFactory, MonomialMatrix, CommutatorMatrixIndex>);
    using AnticommutatorMatrixIndices = MappedMatrixIndices<MonomialMatrix, CommutatorMatrixIndex,
                                                            MonomialAnticommutatorMatrixFactory, PauliMatrixSystem>;

}