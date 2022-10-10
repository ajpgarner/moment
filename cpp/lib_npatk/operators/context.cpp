/**
 * context.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"
#include "operator_sequence.h"

#include <iostream>
#include <sstream>
#include <utility>

namespace NPATK {

    Context::Context(size_t operator_count) {
        this->operators.reserve(operator_count);

        for (size_t index = 0; index < operator_count; ++index) {
            this->operators.emplace_back(static_cast<oper_name_t>(index));
        }

    }

    bool Context::additional_simplification(std::vector<oper_name_t>& op_sequence) const {
        // Do nothing
        return false;
    }

    size_t Context::hash(const OperatorSequence &sequence) const noexcept {
        if (sequence.zero()) {
            return 0;
        }

        return hash(sequence.constituents);
    }

    size_t Context::hash(const std::vector<oper_name_t>& rawOperators) const noexcept {

        size_t hash = 1;
        size_t multiplier = 1;
        const size_t multiplier_stride = 1 + this->operators.size();
        const size_t len = rawOperators.size();

        for (size_t n = 0; n < len; ++n) {
            const auto& oper = rawOperators[len-n-1];
            hash += ((static_cast<size_t>(oper) + 1) * multiplier);
            multiplier *= multiplier_stride;
        }
        return hash;
    }


    std::string Context::format_sequence(const OperatorSequence &seq) const {
        // NB: May be overridden by subclasses!

        if (seq.zero()) {
            return "0";
        }
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;
        bool done_once = false;
        for (const auto& oper : seq) {
            if (done_once) {
                ss << ";";
            } else {
                done_once = true;
            }

            ss << "X" << oper;
        }
        return ss.str();
    }


    std::string Context::to_string() const {
        // NB: May be overridden by subclasses!
        std::stringstream ss;

        ss << "Generic setting.\n";

        const size_t total_operator_count = this->operators.size();
        ss << total_operator_count << ((total_operator_count == 1) ? " operator" : " operators")
           << " in total.\n";

        return ss.str();
    }

    std::ostream &operator<<(std::ostream &os, const Context &context) {
        os << context.to_string();
        return os;
    }



}