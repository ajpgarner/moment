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

    Context::Context(const size_t count)
        : operator_count{static_cast<oper_name_t>(count)}, hasher{count} {

    }

    bool Context::additional_simplification(std::vector<oper_name_t>& op_sequence, bool& negate) const {
        // Do nothing
        return false;
    }

    size_t Context::hash(const OperatorSequence &sequence) const noexcept {
        if (sequence.zero()) {
            return 0;
        }

        return this->hasher(sequence.operators);
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
        if (seq.negated()) {
            ss << "-";
        }

        for (const auto& oper : seq) {
            if (done_once) {
                ss << ";";
            } else {
                done_once = true;
            }

            ss << "X" << (oper+1); // MATLAB indexing...
        }
        return ss.str();
    }


    std::string Context::to_string() const {
        // NB: May be overridden by subclasses!
        std::stringstream ss;

        ss << "Generic setting.\n";

        ss << this->operator_count << ((this->operator_count == 1) ? " operator" : " operators")
           << " in total.\n";

        return ss.str();
    }

    std::ostream &operator<<(std::ostream &os, const Context &context) {
        os << context.to_string();
        return os;
    }



}