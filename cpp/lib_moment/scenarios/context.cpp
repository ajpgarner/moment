/**
 * context.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "context.h"
#include "dictionary/operator_sequence.h"
#include "dictionary/dictionary.h"

#include "symbolic/symbol_table.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

namespace Moment {

    Context::Context(const size_t count)
        : operator_count{static_cast<oper_name_t>(count)}, hasher{count} {
        this->word_list = std::make_unique<Dictionary>(*this);
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
        return OperatorSequence{std::move(str), *this, seq.is_negated};
    }

    OperatorSequence Context::simplify_as_moment(OperatorSequence &&seq) const {
        // Pass through
        return std::move(seq);
    }

    bool Context::can_be_simplified_as_moment(const OperatorSequence &seq) const {
        if (!this->can_have_aliases()) {
            return false;
        }

        auto compare = this->simplify_as_moment(OperatorSequence{seq});
        return compare.hash() != seq.hash();
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

    std::optional<OperatorSequence> Context::get_if_canonical(const sequence_storage_t &raw_sequence) const {
        std::optional<OperatorSequence> output = std::make_optional<OperatorSequence>(raw_sequence, *this);
        // Don't insert sequence if it is the wrong length
        if (output->size() != raw_sequence.size()) {
            return std::nullopt;
        }

        // Don't insert sequence if its hash does not match its raw hash
        const auto raw_hash = this->hash(raw_sequence);
        if (output->hash() != raw_hash) {
            return std::nullopt;
        }

        // Otherwise, return
        return output;
    }

    std::string Context::format_sequence(const OperatorSequence &seq) const {
        std::stringstream ss;
        ContextualOS cSS{ss, *this};
        this->format_sequence(cSS, seq);
        return ss.str();
    }

    void Context::format_sequence(ContextualOS& contextual_os, const OperatorSequence &seq) const {
        // NB: May be overridden by subclasses!
        if (seq.zero()) {
            contextual_os.os << "0";
            return;
        }
        if (seq.empty()) {
            if (seq.negated()) {
                contextual_os.os << "-";
            }
            contextual_os.os << "1";
            return;
        }

        std::stringstream ss;
        bool done_once = false;
        if (seq.negated()) {
            contextual_os.os << "-";
        }

        if (contextual_os.format_info.show_braces) {
            contextual_os.os << "<";
        }
        for (const auto& oper : seq) {
            if (done_once) {
                contextual_os.os << ";";
            } else {
                done_once = true;
            }

            contextual_os.os << "X" << (oper+1); // MATLAB indexing...
        }
        if (contextual_os.format_info.show_braces) {
            contextual_os.os << ">";
        }
    }

    std::string Context::format_raw_sequence(const sequence_storage_t &seq) const {
        std::stringstream ss;
        ContextualOS cSS{ss, *this};
        this->format_raw_sequence(cSS, seq);
        return ss.str();
    }

    void Context::format_raw_sequence(ContextualOS& contextual_os, const sequence_storage_t &seq) const {
        if (seq.empty()) {
            contextual_os.os << "1";
            return;
        }

        if (contextual_os.format_info.show_braces) {
            contextual_os.os << "<";
        }
        bool done_once = false;
        for (const auto& oper : seq) {
            if (done_once) {
                contextual_os.os << ";";
            } else {
                done_once = true;
            }

            contextual_os.os << "X" << (oper+1); // MATLAB indexing...
        }
        if (contextual_os.format_info.show_braces) {
            contextual_os.os << ">";
        }
    }



    void Context::format_sequence_from_symbol_id(ContextualOS& contextual_os, const symbol_name_t symbol_id,
                                                 const bool conjugated) const {
        // Default behaviour does not know how to do this, and so displays as symbol (with hash)

        if (contextual_os.format_info.hash_before_symbol_id) {
            contextual_os.os << "#";
        }
        contextual_os.os << symbol_id;
        if (conjugated) {
            contextual_os.os << "*";
        }
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
        return std::make_unique<OperatorSequenceGenerator>(*this, word_length);
    }

    std::ostream &operator<<(std::ostream &os, const Context &context) {
        os << context.to_string();
        return os;
    }



}