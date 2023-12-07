/**
 * pauli_osg.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_osg.h"
#include "pauli_context.h"
#include "lattice_duplicator.h"

#include "utilities/combinations.h"
#include "dictionary/multi_operator_iterator.h"

namespace Moment::Pauli {
    namespace {
        /**
         * Calculates nearest neighbour sequences in a chain, with and without wrapping.
         * @tparam wrapped True, to wrap around qubits.
         */
        template<bool wrapped>
        size_t chain_nn_sequences(std::vector<OperatorSequence>& sequences,
                                  const PauliContext& context,
                                  const size_t word_length) {
            size_t init_size = sequences.size();

            if (word_length >= 2) [[likely]] {
                LatticeDuplicator ld{context, sequences};
                for (size_t first_party = 0; first_party <= context.qubit_size-2; ++ first_party) {
                    ld.two_qubit_fill(first_party, first_party+1);
                }
                if constexpr(wrapped) {
                    ld.two_qubit_fill(context.qubit_size-1, 0);
                }
            }

            for (size_t parties = 3; parties <= word_length; ++parties) {
                const size_t final_first_party = wrapped ? (context.qubit_size - 1) : (context.qubit_size - parties);
                for (size_t first_party = 0; first_party <= final_first_party; ++ first_party) {
                    //ld.permutation_fill()
                    MultiOperatorIterator pauliIter{context, parties, 3, 0};
                    while (pauliIter) {

                        auto& raw = pauliIter.raw();
                        sequence_storage_t seq_data;
                        seq_data.reserve(parties);

                        for (size_t p_index = 0; p_index < parties; ++p_index) {
                            const size_t party_index = wrapped ? ((first_party+p_index) % context.qubit_size)
                                                               : (first_party+p_index);
                            seq_data.emplace_back(static_cast<oper_name_t>(3 * (party_index) + raw[p_index]));
                        }
                        sequences.emplace_back(std::move(seq_data), context);

                        ++pauliIter;
                    }
                }
            }
            return sequences.size() - init_size;
        }

        /**
         * Calculates next-N nearest neighbour sequences in a chain, with and without wrapping
         * @tparam wrapped True, to wrap around qubits.
         */
        template<bool wrapped>
        size_t chain_next_nn_sequences(std::vector<OperatorSequence>& sequences, const PauliContext& context,
                                       const size_t word_length, const size_t max_distance) {
            assert(max_distance > 1);

            LatticeDuplicator ld{context, sequences};

            size_t init_size = sequences.size();

            for (size_t parties = 2; parties <= word_length; ++parties) {
                const size_t final_first_party = wrapped ? (context.qubit_size - 1) : (context.qubit_size - parties);

                // Prepare  memory to store offsets and resolution into parties
                SmallVector<size_t, 4> selected_parties(parties, static_cast<oper_name_t>(0));

                for (size_t first_party = 0; first_party <= final_first_party; ++ first_party) {
                    MultiOperatorIterator offsetIter{context, parties-1, static_cast<oper_name_t>(max_distance), 1};

                    selected_parties[0] = first_party;

                    while (offsetIter) {
                        const auto& raw_offset = offsetIter.raw();

                        bool skip = false;
                        size_t cumulative_offset = 0;
                        for (size_t c = 1; !skip && (c < parties); ++c) {
                            cumulative_offset += raw_offset[c - 1];
                            if constexpr (wrapped) {
                                selected_parties[c] = (first_party + cumulative_offset) % context.qubit_size;
                                if (cumulative_offset >= context.qubit_size) {
                                    skip = true;
                                }
                            } else {
                                selected_parties[c] = first_party + cumulative_offset;
                                if (selected_parties[c] >= context.qubit_size) {
                                    skip = true;
                                }
                            }
                        }

                        // We have valid parties, so generate all Pauli matrices thereof
                        if (!skip) {
                            ld.permutation_fill(selected_parties);
                        } // !skip
                        ++offsetIter;
                    }
                }
            }

            return sequences.size() - init_size;
        }

        /** Calculates nearest neighbour sequences, with and without wrapping for pairs */
        template<bool wrapped>
        size_t lattice_neighbour_pairs(std::vector<OperatorSequence>& sequences, const PauliContext& context) {
            const auto init_size = sequences.size();

            LatticeDuplicator ld{context, sequences};

            // Iterate over all qubits in lattice
            oper_name_t qubit_index = 0;
            // Major index is column index; and there are as many columns as the width of 1 row
            for (oper_name_t col_id = 0, max_col = context.row_width - 1; col_id < max_col; ++col_id) {
                // Then each element in a given column is identified by its row index
                // There are as many rows as the height of 1 column.
                for (oper_name_t row_id = 0, max_row = context.col_height - 1; row_id < max_row; ++row_id) {
                    // Add vertical link (within column):
                    ld.two_qubit_fill(qubit_index, qubit_index + 1);
                    // Add horizontal link (to next column):
                    ld.two_qubit_fill(qubit_index, qubit_index + context.col_height);

                    ++qubit_index;
                }

                if constexpr (wrapped) {
                    // Add extra vertical link from bottom of column to top of same column:
                    ld.two_qubit_fill(qubit_index, qubit_index + 1 - context.col_height);
                };
                // Add horizontal link from bottom of column to bottom of next column:
                ld.two_qubit_fill(qubit_index, qubit_index + context.col_height);
                ++qubit_index;
            }

            // Now, add links in final column (iterating over row to identify elements):
            for (oper_name_t row_id = 0, max_row = context.col_height - 1; row_id < max_row; ++row_id) {
                // Add vertical link within final column:
                ld.two_qubit_fill(qubit_index, qubit_index + 1);
                if constexpr (wrapped) {
                    // Add horizontal link from right-most column to corresponding element in left-most column:
                    ld.two_qubit_fill(qubit_index, row_id);
                }
                ++qubit_index;
            }

            // In wrap mode, bottom right element needs links too
            if constexpr (wrapped) {
                // Vertical link from bottom of last column to top of last column:
                ld.two_qubit_fill(qubit_index, qubit_index + 1 - context.col_height);
                // Horizontal link from bottom of last column to bottom of first column:
                ld.two_qubit_fill(qubit_index, context.col_height-1);
            }
            ++qubit_index;

            // Make sure we've covered every lattice site
            assert(qubit_index  == context.qubit_size);
            return sequences.size() - init_size;
        }

        template<bool wrapped>
        size_t lattice_neighbour_triplets(std::vector<OperatorSequence>& sequences, const PauliContext& context) {
            if constexpr(!wrapped) {
                throw std::runtime_error{"Currently, nearest-neighbour triplets are only supported with wrapping."};
            }

            // TODO
            throw std::runtime_error{"Nearest-neighbour triplets are not yet supported."};

        }

        /** Calculates nearest neighbour sequences, with and without wrapping */
        template<bool wrapped>
        size_t add_lattice_neighbours(std::vector<OperatorSequence>& sequences, const PauliContext& context,
                                      const size_t word_length) {
            const auto init_size = sequences.size();

            assert(word_length >= 2);
            lattice_neighbour_pairs<wrapped>(sequences, context);

            if (word_length >= 3) {
                lattice_neighbour_triplets<wrapped>(sequences, context);
            }

            if (word_length >= 4) {
                throw std::runtime_error{"Currently only nearest-neighbour pairs and triplets are supported in 2D."};
            }

            return sequences.size() - init_size;
        }
    }

    PauliSequenceGenerator::PauliSequenceGenerator(const PauliContext& pauli_context, const size_t word_length)
        : OperatorSequenceGenerator{pauli_context, word_length, std::vector<OperatorSequence>{}},
          pauli_context{pauli_context}, nearest_neighbour_index{word_length, 0} {
        this->compute_all_sequences();
    }

    PauliSequenceGenerator::PauliSequenceGenerator(const PauliContext& pauli_context,
                                                   const NearestNeighbourIndex& index)
       : OperatorSequenceGenerator{pauli_context, index.moment_matrix_level, std::vector<OperatorSequence>{}},
         pauli_context{pauli_context}, nearest_neighbour_index{index} {

        if (this->nearest_neighbour_index.neighbours != 0) {
            this->compute_nearest_neighbour_sequences();
        } else {
            this->compute_all_sequences();
        }
    }

    void PauliSequenceGenerator::add_length_zero_sequence() {
        this->unique_sequences.emplace_back(OperatorSequence::Identity(this->pauli_context));
    }

    void PauliSequenceGenerator::add_length_one_sequences() {
        // Add length-1 operators directly
        const auto max_ops = static_cast<oper_name_t>(context.size());
        for (oper_name_t o = 0, oMax = max_ops; o < oMax; ++o) {
            this->unique_sequences.emplace_back(OperatorSequence::ConstructRawFlag{},
                                                sequence_storage_t{o}, context.the_hasher().hash(o), context,
                                                SequenceSignType::Positive);
        }
    }

    void PauliSequenceGenerator::compute_all_sequences() {
        // Cap word length at number of qubits
        const size_t word_length = std::min(this->nearest_neighbour_index.moment_matrix_level,
                                      static_cast<size_t>(this->pauli_context.qubit_size));

        // Create sequence vector, with ID element.
        this->add_length_zero_sequence();

        // Early exit if no non-trivial sequences
        if (word_length < 1) {
            return;
        }

        // Add 1-operator sequences first (using fast algorithm)
        this->add_length_one_sequences();

        // Early exit if no length 2 sequences
        if (word_length < 2) {
            return;
        }

        assert(this->pauli_context.qubit_size > 1);

        // Use lattice duplicator utility for remaining sequences...
        LatticeDuplicator ld{this->pauli_context, this->unique_sequences};

        // Length 2 sequences
        for (size_t qubit_one = 0; qubit_one < this->pauli_context.qubit_size - 1; ++qubit_one) {
            for (size_t qubit_two = qubit_one + 1; qubit_two < this->pauli_context.qubit_size; ++qubit_two) {
                ld.two_qubit_fill(qubit_one, qubit_two);
            }
        }

        // Length 3 sequences, and longer
        for (size_t parties = 3; parties <= word_length; ++parties) {
            PartitionIterator partition{static_cast<size_t>(this->pauli_context.qubit_size), parties};
            while (!partition.done()) {
                ld.permutation_fill(partition.primary());
                ++partition;
            }
        }
    }

    void PauliSequenceGenerator::compute_nearest_neighbour_sequences() {
        assert(nearest_neighbour_index.neighbours > 0);

        const bool wrap = pauli_context.wrap;
        if (pauli_context.is_lattice() && (nearest_neighbour_index.neighbours > 1)) {
            throw std::runtime_error{"Only nearest-neighbour and glass mode are currently supported for 2D lattices."};
        }

        // Cap word length at number of qubits
        const size_t word_length = std::min(this->nearest_neighbour_index.moment_matrix_level,
                                            static_cast<size_t>(this->pauli_context.qubit_size));

        // Create sequence vector, with ID element.
        this->add_length_zero_sequence();

        // Early exit for trivial length-0 case.
        if (word_length < 1) {
            return;
        }

        // Length-1 cases are always the same (include everything):
        this->add_length_one_sequences();

        // Early exit for length-1 case
        if (word_length < 2) {
            return;
        }

        // Sequences of length-2 and higher require careful treatment:
        if (1 == this->nearest_neighbour_index.neighbours) {
            // Special case for nearest neighbours:
            if (pauli_context.is_lattice()) {
                if (wrap) {
                    add_lattice_neighbours<true>(this->unique_sequences, pauli_context, word_length);
                } else {
                    add_lattice_neighbours<false>(this->unique_sequences, pauli_context, word_length);
                }
            } else {
                if (wrap) {
                    chain_nn_sequences<true>(this->unique_sequences, pauli_context, word_length);
                } else {
                    chain_nn_sequences<false>(this->unique_sequences, pauli_context, word_length);
                }
            }
        } else {
            // Should have already thrown exception...
            assert(!pauli_context.is_lattice());

            // General N-nearest cases:
            if (wrap) {
                chain_next_nn_sequences<true>(this->unique_sequences, pauli_context,
                                              word_length, this->nearest_neighbour_index.neighbours);
            } else {
                chain_next_nn_sequences<false>(this->unique_sequences, pauli_context,
                                               word_length, this->nearest_neighbour_index.neighbours);
            }
        }
    }


}