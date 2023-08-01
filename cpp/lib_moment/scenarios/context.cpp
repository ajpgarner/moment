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

    std::string Context::format_raw_sequence(const sequence_storage_t &seq) const {
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

            ss << "X" << (oper+1); // MATLAB indexing...
        }
        return ss.str();
    }

    std::string Context::format_symbol(const SymbolTable &table, const symbol_name_t symbol_id) const {
        std::stringstream ss;
        this->format_symbol(ss, table, symbol_id);
        return ss.str();
    }

    void Context::format_symbol(std::ostream &os, const SymbolTable &table, const symbol_name_t symbol_id) const {
        if (symbol_id < 0 || symbol_id >= table.size()) {
            os << "[UNK: #" << symbol_id << "]";
        } else {
            os << "#" << symbol_id;
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