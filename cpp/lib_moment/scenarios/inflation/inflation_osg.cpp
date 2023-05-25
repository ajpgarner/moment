/**
 * inflation_osg.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_osg.h"

#include "inflation_context.h"
#include "utilities/triangular_index_iterator.h"

namespace Moment::Inflation {

    InflationOperatorSequenceGenerator::InflationOperatorSequenceGenerator(const InflationContext &inflationContext,
                                                                           size_t word_length)
       : OperatorSequenceGenerator{inflationContext, word_length, std::vector<OperatorSequence>{}},
         inflationContext{inflationContext} {



        // Always include identity
        this->unique_sequences.emplace_back(OperatorSequence::Identity(context));

        // Is context such that everything is a projector?
        bool completely_projective =
                std::all_of(inflationContext.Observables().begin(), inflationContext.Observables().end(),
                            [](const auto& obs) {
                    return obs.projective();
                });

        if (completely_projective) {
            this->generate_completely_projective();
        } else {
            this->generate_not_completely_projective();
        }
    }


    void InflationOperatorSequenceGenerator::generate_completely_projective() {
        // Generate commuting sequences, if any
        for (size_t level = 1; level <= this->max_sequence_length; ++level) {
            using unique_osg_iter_t = TriangularIndexIterator<oper_name_t, sequence_storage_t, false>;
            unique_osg_iter_t osg_iter{static_cast<oper_name_t>(context.size()), level};
            while (osg_iter) {

                const auto &sequence = *osg_iter;

                // Test if osg_sequence is canonical
                const bool is_canonical = [this, level](const sequence_storage_t &sequence) {
                    const InflationContext::ICOperatorInfo::IsOrthogonal isOrth;
                    // If any adjacent elements from same measurement, then sequence is not unique
                    for (size_t index = 1; index < level; ++index) {
                        const auto &lhs = inflationContext.operator_info[sequence[index - 1]];
                        const auto &rhs = inflationContext.operator_info[sequence[index]];
                        // A0A1 = 0 -> not canonical
                        if (isOrth(lhs, rhs)) {
                            return false;
                        }
                        // A^2 = A -> not canonical
                        if (lhs.projective && (sequence[index - 1] == sequence[index])) {
                            assert(false);
                        }
                    }
                    return true;
                }(sequence);

                if (is_canonical) {
                    this->unique_sequences.emplace_back(OperatorSequence::ConstructRawFlag{},
                                                        sequence, context.hash(sequence), context);
                }
                ++osg_iter;
            }
        }
    }

    void InflationOperatorSequenceGenerator::generate_not_completely_projective() {
        // Generate commuting sequences, if any
        for (size_t level = 1; level <= this->max_sequence_length; ++level) {
            using osg_iter_t = TriangularIndexIterator<oper_name_t, sequence_storage_t, true>;
            osg_iter_t osg_iter{static_cast<oper_name_t>(context.size()), level};
            while (osg_iter) {

                const auto& sequence = *osg_iter;


                // Test if osg_sequence is canonical
                const bool is_canonical = [this, level](const sequence_storage_t& sequence) {
                    const InflationContext::ICOperatorInfo::IsOrthogonal isOrth;
                    // If any adjacent elements from same measurement, then sequence is not unique
                    for (size_t index = 1; index < level; ++index) {
                        const auto &lhs = inflationContext.operator_info[sequence[index - 1]];
                        const auto &rhs = inflationContext.operator_info[sequence[index]];
                        // A0A1 = 0 -> not canonical
                        if (isOrth(lhs, rhs)) {
                            return false;
                        }
                        // A^2 = A -> not canonical
                        if (lhs.projective && (sequence[index - 1] == sequence[index])) {
                            return false;
                        }
                    }
                    return true;
                }(sequence);

                if (is_canonical) {
                    this->unique_sequences.emplace_back(OperatorSequence::ConstructRawFlag{},
                                                        sequence, context.hash(sequence), context);
                }
                ++osg_iter;
            }
        }
    }
}