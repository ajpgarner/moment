/**
 * pauli_osg.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_osg.h"
#include "pauli_context.h"

#include "utilities/combinations.h"
#include "dictionary/multi_operator_iterator.h"

namespace Moment::Pauli {
    namespace {


        /** Adds all 'sequences' consisting of just a single operator. */
        size_t add_length_one_sequences(std::vector<OperatorSequence>& output, const PauliContext& context) {
            // Add length-1 operators directly
            const auto max_ops = static_cast<oper_name_t>(context.size());
            output.reserve(output.size() + max_ops);
            for (oper_name_t o = 0, oMax = max_ops; o < oMax; ++o) {
                output.emplace_back(OperatorSequence::ConstructRawFlag{},
                                    sequence_storage_t{o}, context.the_hasher().hash(o), context,
                                    SequenceSignType::Positive);
            }
            return context.size();
        }

        /** Adds all 'sequences' between a pair of qubits */
        size_t add_correlators_for_pair(std::vector<OperatorSequence>& output, const PauliContext& context,
                                        oper_name_t qubitA, oper_name_t qubitB) {
            // Add length-1 operators directly
            const oper_name_t baseA = 3 * qubitA;
            const oper_name_t baseB = 3 * qubitB;
            for (oper_name_t sigmaA = 0; sigmaA < 3; ++sigmaA) {
                for (oper_name_t sigmaB = 0; sigmaB < 3; ++sigmaB) {
                    output.emplace_back(sequence_storage_t{static_cast<oper_name_t>(baseA + sigmaA),
                                                           static_cast<oper_name_t>(baseB + sigmaB)}, context);
                }
            }
            return 9;
        }


        /** Calculates all sequences in OSG */
        [[nodiscard]] std::vector<OperatorSequence> compute_all_sequences(const PauliContext& context,
                                                                          size_t word_length) {
            // Cap word length at number of qubits
            word_length = std::min(word_length, static_cast<size_t>(context.qubit_size));

            // Create sequence vector, with ID element.
            std::vector<OperatorSequence> sequences;
            sequences.emplace_back(context);

            // Early exit if no non-trivial sequences
            if (word_length < 1) {
                return sequences;
            }

            // Add 1-operator sequences first (using fast algorithm)
            add_length_one_sequences(sequences, context);

            // Then iterate through all ordered multi-partite combinations.
            for (size_t parties = 2; parties <= word_length; ++parties) {
                PartitionIterator partition{static_cast<size_t>(context.qubit_size), parties};
                while (!partition.done()) {
                    MultiOperatorIterator pauliIter{context, parties, 3, 0};
                    while (pauliIter) {

                        auto& raw = pauliIter.raw();
                        sequence_storage_t seq_data;
                        seq_data.reserve(parties);

                        for (size_t p_index = 0; p_index < parties; ++p_index) {
                            seq_data.emplace_back(static_cast<oper_name_t >((partition.primary(p_index)*3)
                                                                            + raw[p_index]));
                        }
                        sequences.emplace_back(std::move(seq_data), context);

                        ++pauliIter;
                    }
                    ++partition;
                }
            }
            return sequences;
        }



        /** Calculates nearest neighbour sequences, with and without wrapping */
        template<bool wrapped>
        size_t add_adjacent_sequences(std::vector<OperatorSequence>& sequences, const PauliContext& context,
                                      const size_t word_length) {
            size_t init_size = sequences.size();

            for (size_t parties = 2; parties <= word_length; ++parties) {
                const size_t final_first_party = wrapped ? (context.qubit_size - 1) : (context.qubit_size - parties);
                for (size_t first_party = 0; first_party <= final_first_party; ++ first_party) {
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
         * Calculates next-N nearest neighbour sequences.
         * @tparam wrap True, to wrap around qubits.
         */
        template<bool wrapped>
        size_t add_nontrival_nnn_sequences(std::vector<OperatorSequence>& sequences, const PauliContext& context,
                                              const size_t word_length, const size_t max_distance) {
            assert(max_distance > 1);

            size_t init_size = sequences.size();

            for (size_t parties = 2; parties <= word_length; ++parties) {
                const size_t final_first_party = wrapped ? (context.qubit_size - 1) : (context.qubit_size - parties);

                // Prepare  memory to store offsets and resolution into parties
                SmallVector<oper_name_t, 4> selected_parties(parties, static_cast<oper_name_t>(0));

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
                            // Generate all Pauli matrices for these parties
                            MultiOperatorIterator pauliIter{context, parties, 3, 0};
                            while (pauliIter) {
                                auto& raw_pauli = pauliIter.raw();
                                sequence_storage_t seq_data;
                                seq_data.reserve(parties);

                                for (size_t p_index = 0; p_index < parties; ++p_index) {
                                    seq_data.emplace_back(static_cast<oper_name_t>((3 * selected_parties[p_index])
                                                                                    + raw_pauli[p_index]));
                                }
                                sequences.emplace_back(std::move(seq_data), context);
                                ++pauliIter;
                            }
                        } // !skip
                        ++offsetIter;
                    }
                }
            }

            return sequences.size() - init_size;
        }



        /** Calculates nearest neighbour sequences, with and without wrapping for pairs */
        template<bool wrapped>
        size_t add_lattice_neighbour_pairs(std::vector<OperatorSequence>& sequences, const PauliContext& context) {
            const auto init_size = sequences.size();

            // Iterate over all qubits in lattice
            oper_name_t qubit_index = 0;
            // Major index is column index; and there are as many columns as the width of 1 row
            for (oper_name_t col_id = 0, max_col = context.row_width - 1; col_id < max_col; ++col_id) {
                // Then each element in a given column is identified by its row index
                // There are as many rows as the height of 1 column.
                for (oper_name_t row_id = 0, max_row = context.col_height - 1; row_id < max_row; ++row_id) {
                    // Add vertical link (within column):
                    add_correlators_for_pair(sequences, context, qubit_index, qubit_index + 1);
                    // Add horizontal link (to next column):
                    add_correlators_for_pair(sequences, context, qubit_index, qubit_index + context.col_height);

                    ++qubit_index;
                }

                if constexpr (wrapped) {
                    // Add extra vertical link from bottom of column to top of same column:
                    add_correlators_for_pair(sequences, context, qubit_index, qubit_index + 1 - context.col_height);
                };
                // Add horizontal link from bottom of column to bottom of next column:
                add_correlators_for_pair(sequences, context, qubit_index, qubit_index + context.col_height);
                ++qubit_index;
            }

            // Now, add links in final column (iterating over row to identify elements):
            for (oper_name_t row_id = 0, max_row = context.col_height - 1; row_id < max_row; ++row_id) {
                // Add vertical link within final column:
                add_correlators_for_pair(sequences, context, qubit_index, qubit_index + 1);
                if constexpr (wrapped) {
                    // Add horizontal link from right-most column to corresponding element in left-most column:
                    add_correlators_for_pair(sequences, context, qubit_index, row_id);
                }
                ++qubit_index;
            }

            // In wrap mode, bottom right element needs links too
            if constexpr (wrapped) {
                // Vertical link from bottom of last column to top of last column:
                add_correlators_for_pair(sequences, context, qubit_index, qubit_index + 1 - context.col_height);
                // Horizontal link from bottom of last column to bottom of first column:
                add_correlators_for_pair(sequences, context, qubit_index, context.col_height-1);
            }
            ++qubit_index;

            // Make sure we've covered every lattice site
            assert(qubit_index  == context.qubit_size);
            return sequences.size() - init_size;
        }

        /** Calculates nearest neighbour sequences, with and without wrapping */
        template<bool wrapped>
        size_t add_lattice_neighbours(std::vector<OperatorSequence>& sequences, const PauliContext& context,
                                      const size_t word_length) {
            const auto init_size = sequences.size();

            assert(word_length >= 2);
            add_lattice_neighbour_pairs<wrapped>(sequences, context);

            if (word_length >= 3) {
                throw std::runtime_error{"Currently only nearest-neighbour pairs are supported in 2D."};
            }

            return sequences.size() - init_size;
        }

        /** Calculates nearest-neighbours in OSG */
        [[nodiscard]] std::vector<OperatorSequence>
        compute_nn_sequences(const PauliContext& context, size_t word_length,
                             const size_t nearest_neighbours) {
            // Case 0 defaults to all sequences
            if (0 == nearest_neighbours) {
                return compute_all_sequences(context, word_length);
            }

            const bool wrap = context.wrap;

            if (context.is_lattice() && (nearest_neighbours>1)) {
                throw std::runtime_error{"Only nearest-neighbour and glass mode are supported for 2D lattices."};
            }

            // Cap word length at number of qubits
            word_length = std::min(word_length, static_cast<size_t>(context.qubit_size));

            // Create sequence vector, with ID element.
            std::vector<OperatorSequence> sequences;
            sequences.emplace_back(context);

            // Early exit for trivial length-0 case.
            if (word_length < 1) {
                return sequences;
            }

            // Length-1 cases are always the same (include everything):
            add_length_one_sequences(sequences, context);

            // Early exit for length-1 case
            if (word_length < 2) {
                return sequences;
            }

            // Sequences of length-2 and higher require careful treatment:
            if (1 == nearest_neighbours) {
                // Special case for nearest neighbours:
                if (context.is_lattice()) {
                    if (wrap) {
                        add_lattice_neighbours<true>(sequences, context, word_length);
                    } else {
                        add_lattice_neighbours<false>(sequences, context, word_length);
                    }
                } else {
                    if (wrap) {
                        add_adjacent_sequences<true>(sequences, context, word_length);
                    } else {
                        add_adjacent_sequences<false>(sequences, context, word_length);
                    }
                }
            } else {
                // Should have already thrown exception...
                assert(!context.is_lattice());

                // General N-nearest cases:
                if (wrap) {
                    add_nontrival_nnn_sequences<true>(sequences, context, word_length, nearest_neighbours);
                } else {
                    add_nontrival_nnn_sequences<false>(sequences, context, word_length, nearest_neighbours);
                }
            }
            return sequences;
        }
    }

    PauliSequenceGenerator::PauliSequenceGenerator(const PauliContext& pauli_context, const size_t word_length)
        : OperatorSequenceGenerator{pauli_context, word_length, compute_all_sequences(pauli_context, word_length)},
            pauliContext{pauli_context}, nearest_neighbour_index{word_length, 0} { }


    PauliSequenceGenerator::PauliSequenceGenerator(const PauliContext& pauli_context,
                                                   const NearestNeighbourIndex& index)
       : OperatorSequenceGenerator{pauli_context, index.moment_matrix_level,
                                   compute_nn_sequences(pauli_context, index.moment_matrix_level, index.neighbours)},
            pauliContext{pauli_context}, nearest_neighbour_index{index.moment_matrix_level, index.neighbours} {

    }


}