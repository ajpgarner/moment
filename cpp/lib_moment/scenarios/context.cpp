/**
 * context.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "context.h"
#include "operator_sequence.h"
#include "word_list.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

namespace Moment {

    Context::Context(const size_t count)
        : operator_count{static_cast<oper_name_t>(count)}, hasher{count} {
        this->word_list = std::make_unique<WordList>(*this);
    }

    Context::~Context() = default;

    bool Context::additional_simplification(sequence_storage_t &op_sequence, bool& negate) const {
        // Do nothing
        return false;
    }

    OperatorSequence Context::conjugate(const OperatorSequence& seq) const {
        // 0* = 0
        if (seq.zero()) {
            return OperatorSequence::Zero(*this);
        }

        sequence_storage_t str;
        str.reserve(seq.operators.size());
        std::reverse_copy(seq.operators.cbegin(), seq.operators.cend(), std::back_inserter(str));
        return OperatorSequence(std::move(str), *this);
    }

    OperatorSequence Context::simplify_as_moment(OperatorSequence &&seq) const {
        // Pass through
        return std::move(seq);
    }

    size_t Context::hash(const OperatorSequence &sequence) const noexcept {
        if (sequence.zero()) {
            return 0;
        }

        return this->hasher(sequence.operators);
    }


    const OperatorSequenceGenerator&
    Context::operator_sequence_generator(const size_t level, const bool conjugated) const {
        if (conjugated) {
            return this->word_list->conjugated(level);
        }
        return (*this->word_list)[level];
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

    std::unique_ptr<OperatorSequenceGenerator> Context::new_osg(const size_t word_length) const {
        return std::make_unique<OperatorSequenceGenerator>(*this, 0, word_length);
    }

    std::ostream &operator<<(std::ostream &os, const Context &context) {
        os << context.to_string();
        return os;
    }



}