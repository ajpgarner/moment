/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace NPATK {
       InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

        // Create operator and observable info
        this->inflated_observables.reserve(this->base_network.Observables().size());
        this->operator_info.reserve(this->size());
        oper_name_t global_id = 0;
        for (const auto& observable : this->base_network.Observables()) {
            const auto num_copies = static_cast<oper_name_t>(observable.count_copies(inflation_level));
            this->inflated_observables.emplace_back(observable.id, global_id, num_copies);
            for (oper_name_t copy_index = 0; copy_index < num_copies; ++copy_index) {
                for (oper_name_t outcome = 0; outcome < (observable.outcomes-1); ++outcome) {
                    this->operator_info.emplace_back(global_id, observable.id, copy_index, outcome);
                    ++global_id;
                }
            }
        }
        assert(this->operator_info.size() == this->size());
        assert(this->inflated_observables.size() == this->base_network.Observables().size());

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
        std::vector<ICOperatorInfo> io_seq;
        io_seq.reserve(op_sequence.size());
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operators.size())) {
                throw std::range_error{"Operator ID higher than number of known operators."};
            }
            const auto& info = this->operator_info[op];
            io_seq.emplace_back(info);
        }

        // Completely commuting set, so sort (no need for stability)
        std::sort(io_seq.begin(), io_seq.end(), ICOperatorInfo::OrderByID{});

        // Check for nullity
        const ICOperatorInfo::IsOrthogonal isOrth;
        for (size_t index = 1, iMax = io_seq.size(); index < iMax; ++index) {
            if (isOrth(io_seq[index-1], io_seq[index])) {
                op_sequence.clear();
                return true;
            }
        }

        // Remove excess idempotent elements.
        auto trim_idem = std::unique(io_seq.begin(), io_seq.end(),
                                     ICOperatorInfo::IsRedundant{});
        io_seq.erase(trim_idem, io_seq.end());

        // Copy sequence to output
        op_sequence.clear();
        op_sequence.reserve(io_seq.size());
        for (const auto& op : io_seq) {
            op_sequence.emplace_back(op.global_id);
        }
        return false;
    }

    oper_name_t InflationContext::operator_number(oper_name_t observable, oper_name_t variant,
                                                  oper_name_t outcome) const noexcept {
       assert((observable >= 0) && (observable < this->inflated_observables.size()));
       assert((observable >= 0) && (observable < this->base_network.Observables().size()));
       const auto& observable_info = this->inflated_observables[observable];
       assert((variant >= 0) && (variant < observable_info.variant_count));
       const auto& base_observable = this->base_network.Observables();
       return static_cast<oper_name_t>(observable_info.operator_offset)
                + (variant * (static_cast<oper_name_t>(this->base_network.Observables()[observable].outcomes)-1))
                + outcome;
   }

}
