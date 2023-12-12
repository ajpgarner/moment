/**
 * lattice_duplicator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "lattice_duplicator.h"

#include "pauli_context.h"

#include "moment_simplifier_wrapping.h"
#include "moment_simplifier_no_wrapping.h"

#include "dictionary/multi_operator_iterator.h"

#include "utilities/small_vector.h"

#include <cassert>

#include <algorithm>
#include <map>

namespace Moment::Pauli {
    namespace {
        void do_permutation_fill(const PauliContext& context, std::vector<OperatorSequence>& output,
                                 const std::span<const size_t> sites) {
            // Then iterate through all ordered multi-partite combinations.
            const size_t parties = sites.size();
            assert(parties > 2);

            MultiOperatorIterator pauliIter{context, parties, 3, 0};
            while (pauliIter) {

                auto& raw = pauliIter.raw();
                sequence_storage_t seq_data;
                seq_data.reserve(parties);
                for (size_t p_index = 0; p_index < parties; ++p_index) {
                    seq_data.emplace_back(static_cast<oper_name_t>((3 * sites[p_index]) + raw[p_index]));
                }
                output.emplace_back(std::move(seq_data), context);

                ++pauliIter;
            }

        }

        template<size_t num_slides>
        std::pair<size_t, size_t>
        do_unaliased_chain_symmetric_fill(LatticeDuplicator& duplicator, std::vector<OperatorSequence>& output,
                                          const SiteHasher<num_slides>& hasher,
                                          const std::span<const size_t> lattice_indices,
                                          size_t max_index) {

            using HashValue = typename SiteHasher<num_slides>::Datum;

            // First, make base elements
            const auto [first_variant, first_variant_end] = duplicator.permutation_fill(lattice_indices);

            // Calculate hashes of base elements
            SmallVector<HashValue, 9> base_hashes;
            base_hashes.reserve(first_variant_end - first_variant);
            for (size_t v = first_variant; v < first_variant_end; ++v) {
                base_hashes.emplace_back(hasher.hash(output[v]));
            }

            // Chain
            for (size_t qubit = 1; qubit < max_index; ++qubit) {
                for (const auto& base_hash : base_hashes) {
                    output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                        hasher.unhash(hasher.cyclic_shift(base_hash, qubit)),
                                        duplicator.context);
                }
            }
            return {first_variant, output.size()};
        }

        template<size_t num_slides>
        std::pair<size_t, size_t>
        do_unaliased_lattice_symmetric_fill(LatticeDuplicator& duplicator, std::vector<OperatorSequence>& output,
                                            const SiteHasher<num_slides>& hasher,
                                            const std::span<const size_t> lattice_indices) {

            using HashValue = typename SiteHasher<num_slides>::Datum;

            // First, make base elements
            const auto [first_variant, first_variant_end] = duplicator.permutation_fill(lattice_indices);

            // Calculate hashes of base elements
            SmallVector<HashValue, 9> base_hashes;
            base_hashes.reserve(first_variant_end - first_variant);
            for (size_t v = first_variant; v < first_variant_end; ++v) {
                base_hashes.emplace_back(hasher.hash(output[v]));
            }

            // Lattice
            for (size_t col = 0; col < hasher.row_width; ++col) {
                for (size_t row = (col != 0) ? 0 : 1; row < hasher.row_width; ++row) {
                    for (const auto& base_hash : base_hashes) {
                        output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                            hasher.unhash(hasher.lattice_shift(base_hash, row, col)),
                                            duplicator.context);
                    }
                }
            }

            return {first_variant, output.size()};
        }

        template<size_t num_slides>
        std::pair<size_t, size_t> do_aliased_chain_symmetric_fill(LatticeDuplicator& duplicator,
                                                                  std::vector<OperatorSequence>& output,
                                                                  const SiteHasher<num_slides>& hasher,
                                                                  const std::span<const size_t> lattice_indices) {
            const size_t chain_length = duplicator.context.qubit_size;
            assert(!lattice_indices.empty());
            const auto [min_elem_iter, max_elem_iter] = std::minmax_element(lattice_indices.begin(),
                                                                            lattice_indices.end());
            const size_t chain_range = (*max_elem_iter - *min_elem_iter);
            if (chain_range < (chain_length/2)) { // strict inequality with rounding of odd qubit
                // Small difference between min and max cannot result in aliases
                return do_unaliased_chain_symmetric_fill(duplicator, output, hasher, lattice_indices, chain_length);
            }

            // If an alias can appear, it will appear for XX...X, and will be associated with a frequency
            sequence_storage_t base_sequence_X;
            base_sequence_X.reserve(lattice_indices.size());
            std::transform(lattice_indices.begin(), lattice_indices.end(), std::back_inserter(base_sequence_X),
                           [](const size_t qubit_index) -> oper_name_t { return 3 * qubit_index; } );

            // If we can prove there are no chain aliases, then do unaliased fill
            const size_t first_alias = hasher.first_chain_alias(hasher(base_sequence_X));
            return do_unaliased_chain_symmetric_fill(duplicator, output, hasher, lattice_indices, first_alias);
        }


        template<size_t num_slides>
        std::pair<size_t, size_t> do_aliased_lattice_symmetric_fill(LatticeDuplicator& duplicator,
                                                                    std::vector<OperatorSequence>& output,
                                                                    const SiteHasher<num_slides>& hasher,
                                                                    const std::span<const size_t> lattice_indices) {

            // Reason from indices if aliasing is completely impossible
            MomentSimplifierNoWrappingLattice nowrap{duplicator.context};
            auto [max_row, max_col] = nowrap.lattice_maximum(lattice_indices);
            const bool no_horz_alias = (max_row < hasher.row_width/2);
            const bool no_vert_alias = (max_col < hasher.column_height/2);
            if (no_horz_alias && no_vert_alias) {
                return do_unaliased_lattice_symmetric_fill(duplicator, output, hasher, lattice_indices);
            }

            // First, make base elements
            assert(!lattice_indices.empty());
            const auto [first_variant, first_variant_end] = duplicator.permutation_fill(lattice_indices);
            assert(first_variant_end > first_variant);

            // Calculate hashes of base elements
            using HashValue = typename SiteHasher<num_slides>::Datum;
            SmallVector<HashValue, 9> base_hashes;
            base_hashes.reserve(first_variant_end - first_variant);
            for (size_t v = first_variant; v < first_variant_end; ++v) {
                base_hashes.emplace_back(hasher.hash(output[v]));
            }
            assert(!base_hashes.empty());

            const HashValue& base_hash = base_hashes[0]; // Element of all Xs

            // Begin hash table
            std::map<HashValue, std::pair<size_t, size_t>> unique_positions;
            unique_positions.insert(std::make_pair(base_hash,
                                                   std::pair{static_cast<size_t>(0), static_cast<size_t>(0)}));

            // Cycle through offsets checking for aliases
            // NB: This is a brute force method that scales with the number of lattice sites.  There may be some
            //     mathematical properties (to do with 2D translational symmetry) that eliminate the need to check every
            //     single site.  If so, there could be a better implementation...!
            const size_t row_width = duplicator.context.row_width; // aka number of columns in one row
            const size_t col_height = duplicator.context.col_height; // aka number of rows in one column
            for (size_t col = 0; col < row_width; ++col) {
                for (size_t row = 0; row < col_height; ++row) {
                    unique_positions.insert(std::make_pair(hasher.lattice_shift(base_hash, row, col),
                                                           std::make_pair(row, col)));
                }
            }

            // Now, add permutations at every unique offset
            for (const auto& [x_hash, offsets] : unique_positions) {
                if ((offsets.first == 0) && (offsets.second == 0)) {
                    continue; // Skip (0,0) that has already been added.
                }

                for (const auto& zero_offset_hash : base_hashes) {
                    output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                        hasher.unhash(hasher.lattice_shift(zero_offset_hash, offsets.first, offsets.second)),
                                        duplicator.context);
                }

            }

            return {first_variant, output.size()};
        }

        /**
         * Cast hasher to appropriate type, and do sweep through values
         * @tparam num_slides Number of slides in the hasher
         * @param duplicator LatticeDuplicator object
         * @param output MUST match duplicator.output. Exposed only due to privacy.
         * @param lattice_sites
         */
        template<size_t num_slides>
        std::pair<size_t, size_t> do_symmetric_fill(LatticeDuplicator& duplicator,
                                                    std::vector<OperatorSequence>& output,
                                                    const std::span<const size_t> lattice_indices,
                                                    const bool check_for_aliases) {
            // Convert hasher to implementation:~
            const auto& moment_simplifier =
                    dynamic_cast<const MomentSimplifierWrapping<num_slides>&>(duplicator.context.moment_simplifier());
            const auto& hasher = moment_simplifier.site_hasher;

            // Invoke appropriate duplicator
            if (duplicator.context.is_lattice()) {
                if (check_for_aliases) {
                    return do_aliased_lattice_symmetric_fill(duplicator, output, hasher, lattice_indices);
                } else {
                    return do_unaliased_lattice_symmetric_fill(duplicator, output, hasher, lattice_indices);
                }
            } else {
                if (check_for_aliases) {
                    return do_aliased_chain_symmetric_fill(duplicator, output, hasher, lattice_indices);
                } else {
                    return do_unaliased_chain_symmetric_fill(duplicator, output, hasher, lattice_indices,
                                                             duplicator.context.qubit_size);
                }
            }
        }
    }

    /** Adds all 'sequences' consisting of just a single operator. */
    void LatticeDuplicator::one_qubit_fill(size_t qubit_index) {
        auto base_oper = static_cast<oper_name_t>(qubit_index * 3);
        output.emplace_back(OperatorSequence::ConstructRawFlag{},
                            sequence_storage_t{base_oper}, context.the_hasher().hash(base_oper),
                            context, SequenceSignType::Positive);
        output.emplace_back(OperatorSequence::ConstructRawFlag{},
                            sequence_storage_t{static_cast<oper_name_t>(base_oper+1)},
                            context.the_hasher().hash(base_oper+1),
                            context, SequenceSignType::Positive);
        output.emplace_back(OperatorSequence::ConstructRawFlag{},
                            sequence_storage_t{static_cast<oper_name_t>(base_oper+2)},
                            context.the_hasher().hash(base_oper+2),
                            context, SequenceSignType::Positive);
    }


    /** Adds all 'sequences' between a pair of qubits */
    void LatticeDuplicator::two_qubit_fill(size_t qubitA, size_t qubitB) {
        // Add all nine permutations:
        assert(qubitA != qubitB);
        const auto baseA = 3 * qubitA;
        const auto baseB = 3 * qubitB;
        if (baseA < baseB) {
            for (size_t operA = baseA; operA < baseA + 3; ++operA) {
                for (size_t operB = baseB; operB < baseB + 3; ++operB) {
                    output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                        sequence_storage_t{static_cast<oper_name_t>(operA),
                                                           static_cast<oper_name_t>(operB)}, context,
                                        SequenceSignType::Positive);
                }
            }
        } else {
            for (size_t operA = baseA; operA < baseA + 3; ++operA) {
                for (size_t operB = baseB; operB < baseB + 3; ++operB) {
                    output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                        sequence_storage_t{static_cast<oper_name_t>(operB),
                                                           static_cast<oper_name_t>(operA)}, context,
                                        SequenceSignType::Positive);
                }
            }
        }
    }


    /**
     * Instantiate all permutations of X, Y, Z for given shape.
     * @return Pair, with the offset of first, and one past the end of last, of operator sequences added.
     */
    std::pair<size_t, size_t> LatticeDuplicator::permutation_fill(const std::span<const size_t> lattice_sites) {
        const size_t initial_size = output.size();

        switch (lattice_sites.size()) {
            case 0:
                output.emplace_back(OperatorSequence::Identity(context));
                break;
            case 1:
                this->one_qubit_fill(lattice_sites[0]);
                break;
            case 2:
                this->two_qubit_fill(lattice_sites[0], lattice_sites[1]);
                break;
            default:
                do_permutation_fill(context, output, lattice_sites);
                break;
        }

        return {initial_size, output.size()};
    }

    std::pair<size_t, size_t> LatticeDuplicator::symmetrical_fill(const std::span<const size_t> lattice_sites,
                                                                  const bool check_for_aliases) {

        // Nothing to do if no lattice sites
        if (lattice_sites.empty()) {
            return {output.size(), output.size()};
        }

        // If context has no wrapping, filling is (much!) easier
        if (this->context.wrap == WrapType::None) {
            return this->wrapless_symmetrical_fill(lattice_sites);
        }

        // Otherwise, we use cyclic hasher to facilitate our duplications
        switch(this->context.moment_simplifier().impl_label) {
            case 1:
                return do_symmetric_fill<1>(*this, this->output, lattice_sites, check_for_aliases);
            case 2:
                return do_symmetric_fill<2>(*this, this->output, lattice_sites, check_for_aliases);
            case 3:
                return do_symmetric_fill<3>(*this, this->output, lattice_sites, check_for_aliases);
            case 4:
                return do_symmetric_fill<4>(*this, this->output, lattice_sites, check_for_aliases);
            case 5:
                return do_symmetric_fill<5>(*this, this->output, lattice_sites, check_for_aliases);
            case 6:
                return do_symmetric_fill<6>(*this, this->output, lattice_sites, check_for_aliases);
            case 7:
                return do_symmetric_fill<7>(*this, this->output, lattice_sites, check_for_aliases);
            case 8:
                return do_symmetric_fill<8>(*this, this->output, lattice_sites, check_for_aliases);
            default:
                throw std::runtime_error{"Cannot invoke symmetrical duplication for this specialization."};
        }
    }

    std::pair<size_t, size_t>
    LatticeDuplicator::wrapless_symmetrical_fill(const std::span<const size_t> lattice_indices) {
        // Do nothing, if filling empty lattice
        if (lattice_indices.empty()) [[unlikely]] {
            return {this->output.size(), this->output.size()};
        }

        // (Doesn't matter if actual context has wrapping: we use this as a utility class):
        MomentSimplifierNoWrappingLattice simplifier{this->context};

        // Determine minimum and maximum offsets
        const auto [number_rows, number_cols] = simplifier.lattice_maximum(lattice_indices);

        const size_t initial_index = this->output.size();

        // Prepare to iterate over lattice
        std::vector<size_t> actual_indices(lattice_indices.size(), 0);
        for (size_t col = 0, max_col = (simplifier.row_width - number_cols); col < max_col; ++col) {
            for (size_t row = 0, max_row = (simplifier.column_height - number_rows); row < max_row; ++row) {

                // Get transformed site indices
                const size_t the_offset = (col * simplifier.column_height) + row;
                std::transform(lattice_indices.begin(), lattice_indices.end(),
                               actual_indices.begin(),
                               [the_offset](const size_t index) {
                    return index + the_offset;
                });

                // Fill sites
                this->permutation_fill(actual_indices);
            }
        }

        // Report range inserted
        return std::pair<size_t, size_t>{initial_index, this->output.size()};
    }

}