/**
 * moment_simplifier_wrapping.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_simplifier.h"
#include "site_hasher.h"
#include "pauli_context.h"

namespace Moment::Pauli {

    /**
     * Simplifier for wrapping classes; parameterized by maximum number of slides.
     * @tparam slides
     */
    template<size_t slides>
    class MomentSimplifierWrapping : public MomentSimplifier {
    public:
        using Datum = typename SiteHasher<slides>::Datum;

        SiteHasher<slides> site_hasher;

    public:
        explicit MomentSimplifierWrapping(const PauliContext& context)
            : MomentSimplifier(context, slides),
              site_hasher{context.is_lattice() ? context.col_height : context.qubit_size,
                         context.is_lattice() ? context.row_width : 1} {

        }


        using MomentSimplifier::canonical_sequence;

        [[nodiscard]] sequence_storage_t canonical_sequence(const std::span<const oper_name_t> input) const final {
            // Find equivalence class
            const auto [smallest_hash, actual_hash] = site_hasher.canonical_hash(input);

            // Operator sequence is already minimal
            if (smallest_hash == actual_hash) {
                // Copy input to output:
                sequence_storage_t output;
                output.reserve(input.size());
                std::copy(input.begin(), input.end(), std::back_inserter(output));
                return output;
            } else {
                // Otherwise, reconstruct operator sequence data from minimal hash value:
                return site_hasher.unhash(smallest_hash);
            }
        }

        /**
         * Tests canonical version of operator sequence
         */
        [[nodiscard]] bool is_canonical(const std::span<const oper_name_t> input) const noexcept final {
            // Find equivalence class
            const auto [smallest_hash, actual_hash] = site_hasher.canonical_hash(input);

            // Is input operator sequence already minimal?
            return (smallest_hash == actual_hash);
        }
    };

}