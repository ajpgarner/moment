/**
 * lattice_duplicator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "lattice_duplicator.h"

#include "pauli_context.h"

#include "site_hasher.h"
#include "site_hasher_impl.h"

#include "dictionary/multi_operator_iterator.h"

#include <cassert>

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
        void do_unaliased_chain_symmetric_fill(LatticeDuplicator& duplicator, std::vector<OperatorSequence>& output,
                                    const SiteHasherImpl<num_slides>& hasher,
                                    const std::span<const typename SiteHasherImpl<num_slides>::Datum> base_hashes) {
            // Chain
            for (size_t qubit = 1; qubit < hasher.qubits; ++qubit) {
                for (const auto& base_hash : base_hashes) {
                    output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                        hasher.unhash(hasher.cyclic_shift(base_hash, qubit)),
                                        hasher.context);
                }
            }
        }

        template<size_t num_slides>
        void do_unaliased_lattice_symmetric_fill(LatticeDuplicator& duplicator, std::vector<OperatorSequence>& output,
                                    const SiteHasherImpl<num_slides>& hasher,
                                    const std::span<const typename SiteHasherImpl<num_slides>::Datum> base_hashes) {
            // Lattice
            for (size_t col = 0; col < hasher.row_width; ++col) {
                for (size_t row = (col != 0) ? 0 : 1; row < hasher.row_width; ++row) {
                    for (const auto& base_hash : base_hashes) {
                        output.emplace_back(OperatorSequence::ConstructPresortedFlag{},
                                            hasher.unhash(hasher.lattice_shift(base_hash, row, col)),
                                            hasher.context);
                    }
                }
            }
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
            const auto& hasher = dynamic_cast<const SiteHasherImpl<num_slides>&>(duplicator.context.site_hasher());
            using Datum = typename SiteHasherImpl<num_slides>::Datum;

            // First, make base elements
            const auto [first_variant, first_variant_end] = duplicator.permutation_fill(lattice_indices);

            // Calculate hashes of base elements
            std::vector<Datum> base_hashes;
            base_hashes.reserve(first_variant_end - first_variant);
            for (size_t v = first_variant; v < first_variant_end; ++v) {
                base_hashes.emplace_back(hasher.hash(output[v]));
            }

            // Invoke appropriate duplicator
            if (hasher.context.is_lattice()) {
                do_unaliased_lattice_symmetric_fill(duplicator, output, hasher, base_hashes);
            } else {
                do_unaliased_chain_symmetric_fill(duplicator, output, hasher, base_hashes);
            }

            // Report number of created elements in total
            return std::make_pair(first_variant, output.size());
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
        if (!this->context.wrap) {
            return this->wrapless_symmetrical_fill(lattice_sites);
        }

        // Otherwise, we use cyclic hasher to facilitate our duplications
        switch(this->context.site_hasher().impl_label) {
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
    LatticeDuplicator::wrapless_symmetrical_fill(std::span<const size_t> lattice_sites) {

        // TODO

        return std::pair<size_t, size_t>(this->output.size(), this->output.size());
    }

}