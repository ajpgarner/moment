/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

#include <algorithm>
#include <sstream>

namespace NPATK {
    namespace {
        struct InflationOperator {
        public:
            oper_name_t id = 0;
            oper_name_t observable_id = 0;
            oper_name_t observable_copy = 0;

            InflationOperator() = default;
            InflationOperator(oper_name_t id, oper_name_t o_id, oper_name_t o_copy)
                : id{id}, observable_id{o_id}, observable_copy{o_copy} { }

            /**
            * Predicate: true if the operator id of LHS is less than that of RHS.
            */
            struct Comparator {
                constexpr bool operator()(const InflationOperator &lhs, const InflationOperator &rhs) const noexcept {
                    return lhs.id < rhs.id;
                }
            };

            /**
             * Predicate: true if lhs != rhs, but both are part of same observable.
             */
            struct IsOrthogonal {
                constexpr bool operator()(const InflationOperator &lhs, const InflationOperator &rhs) const noexcept {
                    // Not in same version of same observable, therefore not automatically orthogonal.
                    if ((lhs.observable_id != rhs.observable_id) || (lhs.observable_copy != rhs.observable_copy)) {
                        return false;
                    }
                    return (lhs.id != rhs.id);
                }
            };

            /**
             * Predicate: true if lhs == rhs, and both are part of same observable.
             */
            struct IsRedundant {
                constexpr bool operator()(const InflationOperator &lhs, const InflationOperator &rhs) const noexcept {
                    return (lhs.id == rhs.id);
                }
            };

        };
    }


    InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

        // Create operator info
        this->operator_info.reserve(this->size());

        oper_name_t global_id = 0;
        for (const auto& observable : this->base_network.Observables()) {
            const auto num_copies = static_cast<oper_name_t>(observable.count_copies(inflation_level));
            for (oper_name_t copy_index = 0; copy_index < num_copies; ++copy_index) {
                for (oper_name_t outcome = 0; outcome < (observable.outcomes-1); ++outcome) {
                    this->operator_info.emplace_back(global_id, observable.id, copy_index, outcome);
                    ++global_id;
                }
            }
        }
        assert(this->operator_info.size() == this->size());

    }

    std::string InflationContext::to_string() const {
        std::stringstream ss;
        ss << "Inflation setting with "
           << this->operators.size() << ((1 !=  this->operators.size()) ? " operators" : " operator")
           << " in total.\n\n";
        ss << this->base_network << "\n";
        ss << "Inflation level: " << this->inflation;

        return ss.str();
    }

    bool InflationContext::additional_simplification(std::vector<oper_name_t> &op_sequence, bool &negate) const {
        // Commutation between parties...
        std::vector<InflationOperator> io_seq;
        io_seq.reserve(op_sequence.size());
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operators.size())) {
                throw std::range_error{"Operator ID higher than number of known operators."};
            }
            io_seq.emplace_back(op, 0, 0); // XXX get actual info for observables..
        }

        // Completely commuting set, so sort
        std::sort(io_seq.begin(), io_seq.end(), InflationOperator::Comparator{});

        return Context::additional_simplification(op_sequence, negate);
    }

}
