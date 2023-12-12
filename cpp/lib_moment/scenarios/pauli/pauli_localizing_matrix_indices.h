/**
 * pauli_localizing_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "dictionary/operator_sequence.h"
#include "matrix_system/matrix_indices.h"

#include "multithreading/maintains_mutex.h"

#include "integer_types.h"
#include "nearest_neighbour_index.h"
#include "matrix_system/localizing_matrix_index.h"

#include <set>
#include <vector>

namespace Moment {
    class MatrixSystem;
    class MonomialMatrix;
};

namespace Moment::Pauli {
    class PauliMatrixSystem;

    struct PauliLocalizingMatrixIndex {
        NearestNeighbourIndex Index;
        OperatorSequence Word;
        uint64_t WordHash;


        PauliLocalizingMatrixIndex(const NearestNeighbourIndex& nn_info, OperatorSequence word)
            : Index{nn_info}, Word{std::move(word)}, WordHash{Word.hash()} {
        }

        PauliLocalizingMatrixIndex(const size_t level, const size_t neighbours, OperatorSequence word)
            : PauliLocalizingMatrixIndex{NearestNeighbourIndex{level, neighbours}, std::move(word)} {
        }

        explicit PauliLocalizingMatrixIndex(LocalizingMatrixIndex lmi)
            : Index{lmi.Level, 0}, Word{std::move(lmi.Word)}, WordHash{lmi.WordHash} {
        }

        PauliLocalizingMatrixIndex(const PauliLocalizingMatrixIndex& rhs) = default;

        PauliLocalizingMatrixIndex(PauliLocalizingMatrixIndex&& rhs) = default;


        [[nodiscard]] bool operator<(const PauliLocalizingMatrixIndex& pmmi) const noexcept {
            // First compare indices
            const auto three_way_index = (this->Index <=> pmmi.Index);
            if (three_way_index < 0) {
                return true;
            } else if (three_way_index > 0) {
                return false;
            }

            // Tie-break on word hash
            return this->WordHash < pmmi.WordHash;
        }

        [[nodiscard]] operator LocalizingMatrixIndex() const {
            return {this->Index.moment_matrix_level, this->Word};
        }

    };

    class PauliLocalizingMatrixFactory final {
    private:
        PauliMatrixSystem& system;

    public:
        using Index = PauliLocalizingMatrixIndex;

        explicit PauliLocalizingMatrixFactory(MatrixSystem& system);

        explicit PauliLocalizingMatrixFactory(PauliMatrixSystem& system) noexcept : system{system} { }

        [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, MonomialMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;
    };

    static_assert(makes_matrices<PauliLocalizingMatrixFactory, MonomialMatrix, PauliLocalizingMatrixIndex>);

    using PauliLocalizingMatrixIndices = MappedMatrixIndices<MonomialMatrix, PauliLocalizingMatrixIndex,
            PauliLocalizingMatrixFactory, PauliMatrixSystem>;

}