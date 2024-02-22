/**
 * monomial_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once


#include "nearest_neighbour_index.h"

#include "dictionary/operator_sequence.h"

#include "matrix_system/indices/localizing_matrix_index.h"
#include "matrix_system/matrix_indices.h"
#include "matrix_system/index_storage/map_index_storage.h"

#include "multithreading/maintains_mutex.h"

#include "integer_types.h"

#include <iosfwd>
#include <string>

namespace Moment {
    class MatrixSystem;
    class MonomialMatrix;
};

namespace Moment::Pauli {
    class PauliMatrixSystem;

    struct PauliMonomialIndex {
    public:
        using OSGIndex = NearestNeighbourIndex;

        NearestNeighbourIndex Index;
        OperatorSequence Word;
        uint64_t WordHash;


        PauliMonomialIndex(const NearestNeighbourIndex& nn_info, OperatorSequence word)
            : Index{nn_info}, Word{std::move(word)}, WordHash{Word.hash()} {
        }

        PauliMonomialIndex(const size_t level, const size_t neighbours, OperatorSequence word)
            : PauliMonomialIndex{NearestNeighbourIndex{level, neighbours}, std::move(word)} {
        }

        PauliMonomialIndex(const PauliMonomialIndex& rhs) = default;

        PauliMonomialIndex(PauliMonomialIndex&& rhs) = default;


        [[nodiscard]] bool operator<(const PauliMonomialIndex& pmmi) const noexcept {
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

        [[nodiscard]] operator ::Moment::LocalizingMatrixIndex() const {
            return {this->Index.moment_matrix_level, this->Word};
        }

    };

    struct LocalizingMatrixIndex : public PauliMonomialIndex {
        explicit LocalizingMatrixIndex(::Moment::LocalizingMatrixIndex plain_info) noexcept
            : PauliMonomialIndex{NearestNeighbourIndex{plain_info.Level, 0}, std::move(plain_info.Word)} { }

        LocalizingMatrixIndex(const NearestNeighbourIndex& nn_info, OperatorSequence word) noexcept
            : PauliMonomialIndex{nn_info, std::move(word)} { }

        LocalizingMatrixIndex(size_t level, size_t neighbours, OperatorSequence word) noexcept
            : PauliMonomialIndex{NearestNeighbourIndex{level, neighbours}, std::move(word)} { }

        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] inline std::string to_string(const PauliMatrixSystem& system) const {
            return this->to_string();
        }
    };

    struct CommutatorMatrixIndex : public PauliMonomialIndex {
        CommutatorMatrixIndex(const NearestNeighbourIndex& nn_info, OperatorSequence word) noexcept
            : PauliMonomialIndex{nn_info, std::move(word)} { }

        explicit CommutatorMatrixIndex(Pauli::LocalizingMatrixIndex plmi) noexcept
            : PauliMonomialIndex{plmi.Index, std::move(plmi.Word)} { }

        CommutatorMatrixIndex(size_t level, size_t neighbours, OperatorSequence word) noexcept
        : PauliMonomialIndex{NearestNeighbourIndex{level, neighbours}, std::move(word)} { }

        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] inline std::string to_string(const PauliMatrixSystem& system) const {
            return this->to_string();
        }
    };

    struct AnticommutatorMatrixIndex : public PauliMonomialIndex {
        AnticommutatorMatrixIndex(const NearestNeighbourIndex& nn_info, OperatorSequence word) noexcept
            : PauliMonomialIndex{nn_info, std::move(word)} { }

        explicit AnticommutatorMatrixIndex(Pauli::LocalizingMatrixIndex plmi) noexcept
            : PauliMonomialIndex{plmi.Index, std::move(plmi.Word)} { }

        AnticommutatorMatrixIndex(size_t level, size_t neighbours, OperatorSequence word) noexcept
        : PauliMonomialIndex{NearestNeighbourIndex{level, neighbours}, std::move(word)} { }

        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] inline std::string to_string(const PauliMatrixSystem& system) const {
            return this->to_string();
        }
    };

}