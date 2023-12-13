/**
 * nonwrapping_simplifier.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_simplifier_no_wrapping.h"
#include "scenarios/pauli/pauli_context.h"

#include <algorithm>

namespace Moment::Pauli {

    MomentSimplifierNoWrappingChain::MomentSimplifierNoWrappingChain(const PauliContext& context)
            : MomentSimplifier{context, MomentSimplifierNoWrappingChain::expected_label},
              qubits{context.qubit_size} { }

    sequence_storage_t MomentSimplifierNoWrappingChain::canonical_sequence(const std::span<const oper_name_t> input) const {
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



    MomentSimplifierNoWrappingLattice::MomentSimplifierNoWrappingLattice(const PauliContext& context)
            : MomentSimplifier{context, MomentSimplifierNoWrappingLattice::expected_label},
              qubits{context.qubit_size},
              column_height{context.is_lattice() ? context.col_height : context.qubit_size},
              row_width{context.is_lattice() ? context.row_width : 1},
              column_op_height{context.col_height*3} {

    }

    sequence_storage_t
    MomentSimplifierNoWrappingLattice::canonical_sequence(const std::span<const oper_name_t> input) const {
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

    bool MomentSimplifierNoWrappingLattice::is_canonical(const std::span<const oper_name_t> input) const noexcept {
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

    std::pair<size_t, size_t>
    MomentSimplifierNoWrappingLattice::lattice_minimum(const std::span<const oper_name_t> input) const {
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

    std::pair<size_t, size_t>
    MomentSimplifierNoWrappingLattice::lattice_maximum(const std::span<const oper_name_t> input) const {
        // Special case for empty:
        if (input.empty()) [[unlikely]] {
            return {this->column_height, this->row_width};
        }

        // Max column will be column of last qubit:
        const size_t max_column = input.back() / this->column_op_height;

        // Scan for maximum row
        size_t max_row = (input[0] / 3) % this->column_height;
        for (size_t idx = 1; idx < input.size(); ++idx) {
            max_row = std::max(max_row, (input[idx] /3) % this->column_height);
        }

        return {max_row, max_column};
    }

    std::pair<size_t, size_t>
    MomentSimplifierNoWrappingLattice::lattice_maximum(const std::span<const size_t> input) const {
        // Special case for empty:
        if (input.empty()) [[unlikely]] {
            return {this->column_height, this->row_width};
        }

        // Max column will be column of last qubit:
        const size_t max_column = input.back() / this->column_height;

        // Scan for maximum row
        size_t max_row = input[0] % this->column_height;
        for (size_t idx = 1; idx < input.size(); ++idx) {
            max_row = std::max(max_row, input[idx]  % this->column_height);
        }

        return {max_row, max_column};
    }

}