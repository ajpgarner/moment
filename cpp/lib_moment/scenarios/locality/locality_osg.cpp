/**
 * locality_osg.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_osg.h"

#include "scenarios/multi_operator_iterator.h"
#include "scenarios/operator_sequence.h"

#include "utilities/multi_partition.h"
#include "utilities/multi_dimensional_index_iterator.h"
#include "utilities/small_vector.h"

#include "locality_context.h"

#include <stdexcept>

namespace Moment::Locality {

    namespace {
        using span_container_t = SmallVector<std::span<const OperatorSequence>, 4>;

        span_container_t get_party_spans(const std::vector<LocalityOperatorSequenceGenerator::PartyOSG> &parties,
                                         const MultipartitionIterator<size_t, true> &mpi) {
            span_container_t output;
            const auto party_size = parties.size();
            assert(mpi.Parties == party_size);
            output.reserve(party_size);
            const auto& sub_lengths = *mpi;

            for (size_t p_idx = 0; p_idx < party_size; ++p_idx) {
                output.emplace_back(parties[p_idx][sub_lengths[p_idx]]);
            }
            return output;
        }

        MultiDimensionalIndexIterator<> make_iterator(const span_container_t& constituents) {
            std::vector<size_t> span_lengths;
            span_lengths.reserve(constituents.size());
            for (auto c : constituents) {
                span_lengths.emplace_back(c.size());
            }
            return MultiDimensionalIndexIterator<>{std::move(span_lengths)};
        }

        void tensor_populate(std::vector<OperatorSequence>& output,
                             const Context& context,
                             const span_container_t& constituents,
                             const size_t target_wl) {


            const size_t party_size = constituents.size();
            auto partition_iter = make_iterator(constituents);

            // Same size sequence for every element, so re-use to avoid re-allocating
            sequence_storage_t next_seq(target_wl, 0);

            while (partition_iter) {
                const auto& indices = *partition_iter;

                // Concatenate:
                ptrdiff_t offset = 0;
                for (size_t p_idx = 0; p_idx < party_size; ++p_idx) {
                    const auto& sub_seq = constituents[p_idx][indices[p_idx]];
                    std::copy(sub_seq.begin(), sub_seq.end(), next_seq.begin() + offset);
                    offset += sub_seq.size();
                }
                assert(offset == target_wl);

                // Make new op sequence
                output.emplace_back(next_seq, context); // <- copies next_seq.

                ++partition_iter;
            }
        }
    }


    LocalityOperatorSequenceGenerator::PartyOSG::PartyOSG(const LocalityContext &context,
                                                          const class Party * party,
                                                          const size_t max_word_length)
        : party{party} {
        assert(party != nullptr);

        // Every party defines an identity [level 0]
        this->unique_sequences.emplace_back(context);
        this->word_length_boundaries.emplace_back(1); // level 0 ends before 1.

        for (size_t wl = 1; wl <= max_word_length; ++wl) {
            // make strings of length wl, and add
            MultiOperatorIterator moi{context, wl,
                                      static_cast<oper_name_t>(party->size()),
                                      static_cast<oper_name_t>(party->global_offset())};
            while (moi) {
                OperatorSequence next{*moi};
                if (wl == next.size()) { // If op seq is too short, do not add, as it will have appeared earlier...
                    this->unique_sequences.emplace_back(std::move(next));
                }
                ++moi;
            }
            this->word_length_boundaries.emplace_back(static_cast<ptrdiff_t>(this->unique_sequences.size()));
        }
    }

    std::span<const OperatorSequence>
    LocalityOperatorSequenceGenerator::PartyOSG::operator[](const size_t word_length) const noexcept {
        assert(word_length < word_length_boundaries.size());

        const ptrdiff_t first_elem = (word_length > 0) ? this->word_length_boundaries[word_length-1] : 0;
        const ptrdiff_t last_elem = this->word_length_boundaries[word_length];
        assert(last_elem >= first_elem);

        return {this->unique_sequences.cbegin() + first_elem, static_cast<size_t>(last_elem - first_elem)};
    }


    LocalityOperatorSequenceGenerator::LocalityOperatorSequenceGenerator(const LocalityContext &context,
                                                                         const size_t the_wl)
            : OperatorSequenceGenerator{context, std::vector<OperatorSequence>{}, 0, the_wl}, localityContext{context} {
        // Step 1: make party OSGs
        this->parties.reserve(localityContext.Parties.size());
        for (const auto& party : localityContext.Parties) {
            this->parties.emplace_back(localityContext, &party, the_wl);
        }

        // Step 2: combine party OSGs to make total OSG
        switch (this->parties.size()) {
            case 0:
                this->populate_zero_parties();
                break;
            case 1:
                this->populate_one_party();
                break;
            default:
                this->populate_general();
                break;
        }

    }

    void LocalityOperatorSequenceGenerator::populate_zero_parties() {
        // When no parties, only sequence is identity.
        this->unique_sequences.emplace_back(context);
    }

    void LocalityOperatorSequenceGenerator::populate_one_party() {
        // For one party, can just copy across all sequences
        assert(this->parties.size() == 1);
        auto view = this->parties[0].all();
        std::copy(view.begin(), view.end(), std::back_inserter(this->unique_sequences));
    }

    void LocalityOperatorSequenceGenerator::populate_general() {
        // Begin with level 0:
        this->unique_sequences.emplace_back(context);
        if (this->max_sequence_length <= 0) {
            return;
        }

        // Level 1, copy each party's operators in turn
        for (const auto& party_osg : this->parties) {
            assert(party_osg.word_length() >= 1);
            auto view = party_osg[1];
            std::copy(view.begin(), view.end(), std::back_inserter(this->unique_sequences));
        }
        if (this->max_sequence_length <= 1) {
            return;
        }

        // Level 2 onwards, interleaving
        for (size_t wl = 2; wl <= this->max_sequence_length; ++wl) {
            MultipartitionIterator<size_t, true> mpi{wl, this->parties.size()};

            while (mpi) {
                auto sub_spans = get_party_spans(this->parties, mpi);
                tensor_populate(this->unique_sequences, this->localityContext, sub_spans, wl);
                ++mpi;
            }
        }
    }


}