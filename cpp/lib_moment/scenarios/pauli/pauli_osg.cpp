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

        [[nodiscard]] std::vector<OperatorSequence> compute_all_sequences(const PauliContext& context,
                                                                          size_t word_length) {
            // Cap word length at number of qubits
            word_length = std::min(word_length, static_cast<size_t>(context.qubit_size));

            // Create sequence vector, with ID element.
            std::vector<OperatorSequence> sequences;
            sequences.emplace_back(context);

            // Ramp up to number of parties.
            for (size_t parties = 1; parties <= word_length; ++parties) {
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
    }

    PauliSequenceGenerator::PauliSequenceGenerator(const PauliContext& pauli_context, const size_t word_length)
        : OperatorSequenceGenerator{pauli_context, word_length, compute_all_sequences(pauli_context, word_length)},
            pauliContext{pauli_context} {

    }
}