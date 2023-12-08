/**
 * nonwrapping_simplifier.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "nonwrapping_simplifier.h"
#include "pauli_context.h"

#include <algorithm>

namespace Moment::Pauli {

    sequence_storage_t NonwrappingChainSimplifier::canonical_sequence(const std::span<const oper_name_t> input) const {
        sequence_storage_t output;
        output.reserve(input.size());

        const oper_name_t oper_offset = 3*this->chain_minimum(input); // will return 0 if input is empty...

        if (0 == oper_offset) [[unlikely]] {
            // No offset possible, just copy:
            std::copy(input.begin(), input.end(), std::back_inserter(output));
        } else {
            // Otherwise, apply constant chain shift:
            std::transform(input.begin(), input.end(), std::back_inserter(output),
                           [oper_offset](const oper_name_t op) -> oper_name_t {
                return op - oper_offset;
            });
        }

        // Return new op sequence
        return output;
    }

    sequence_storage_t NonwrappingLatticeSimplifier::canonical_sequence(const std::span<const oper_name_t> input) const {
        // Can we move into a corner?
        const auto [row_offset, col_offset] = this->lattice_minimum(input);

        // Prepare output
        sequence_storage_t output;
        output.reserve(input.size());


        if ((row_offset == 0) && (col_offset == 0)) {
            // No row offset, no col offset -> copy
            std::copy(input.begin(), input.end(), std::back_inserter(output));
        } else {
            // Otherwise work out by how much to offset each index
            const size_t oper_offset = (col_offset * this->column_op_height) + (row_offset * 3);
            std::transform(input.begin(), input.end(), std::back_inserter(output),
                           [oper_offset](const oper_name_t op) -> oper_name_t  {
               return op - oper_offset; // naively works because we've guaranteed to have no ops on rows < row_offset.
            });
        }
        return output;
    }

    bool NonwrappingLatticeSimplifier::is_canonical(const std::span<const oper_name_t> input) const noexcept {
        // Empty input is always canonical
        if (input.empty()) {
            return true;
        }

        // If first column is not 0, never canonical
        if ((input[0] / this->column_op_height) > 0) {
            return false;
        }

        // If any op is on row 0, we are canonical.
        return std::any_of(input.begin(), input.end(), [this](const oper_name_t op)->bool {
            return (0 == ((op / 3) % this->column_height));
        });
    }

    std::pair<size_t, size_t> NonwrappingLatticeSimplifier::lattice_minimum(const std::span<const oper_name_t> input) const {
        // Special case for empty:
        if (input.empty()) [[unlikely]] {
            return {0, 0};
        }

        // Min column will be column of first qubit:
        const size_t min_column = input[0] / this->column_op_height;

        // Scan for minimum row
        size_t min_row = (input[0] / 3) % this->column_height;
        for (size_t idx = 1; idx < input.size(); ++idx) {
            min_row = std::min(min_row, (input[idx] /3) % this->column_height);
        }

        return {min_row, min_column};
    }
}