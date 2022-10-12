/**
 * raw_sequence_book.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "raw_sequence_book.h"
#include "../context.h"

#include "../multi_operator_iterator.h"

#include <cmath>

namespace NPATK {

    namespace {
        size_t num_permutations(const size_t alphabet, const size_t minLength, const size_t maxLength) {
            size_t output = 0;
            for (size_t i = minLength; i< maxLength; ++i) {
                output += static_cast<size_t>(pow(static_cast<double>(alphabet), static_cast<double>(i)));
            }
            return output;
        }
    }


    RawSequenceBook::RawSequenceBook(const Context& context) : context{context} {
        // Always add zero and one symbols...
        this->sequences.reserve(2);
        this->sequences.emplace_back(std::vector<oper_name_t>{}, 0, 0); // hash of 0 is always 0
        this->sequences.emplace_back(std::vector<oper_name_t>{}, 1, 1); // hash of 1 is always 1
        this->symbols.reserve(2);
        this->symbols.emplace_back(Symbol::zero());
        this->symbols.emplace_back(1);
        this->hash_table.emplace(std::make_pair(0, 0));
        this->hash_table.emplace(std::make_pair(1, 1));

    }

    bool RawSequenceBook::generate(const size_t target_length) {
        // Do nothing if below current length
        if (target_length <= this->max_seq_length) {
            return false;
        }

        // Reserve for new sequences...
        const size_t new_elements = num_permutations(this->context.size(), this->max_seq_length, target_length);
        this->sequences.reserve(this->sequences.size() + new_elements);
        this->symbols.reserve(this->symbols.size() + new_elements);

        auto symbol_id = static_cast<symbol_name_t>(this->sequences.size());

        for (size_t length = this->max_seq_length+1; length <= target_length; ++length) {
            MultiOperatorIterator moi{this->context, length};
            MultiOperatorIterator moi_end{MultiOperatorIterator::end_of(this->context, length)};
            while (moi != moi_end) {
                auto rawStr = moi.raw();
                size_t hash = this->context.hash(rawStr);
                this->sequences.emplace_back(std::move(rawStr), hash, symbol_id);
                this->hash_table.emplace(std::make_pair(hash, symbol_id));
                this->symbols.emplace_back(symbol_id);

                ++symbol_id;
                ++moi;
            }
        }

        this->max_seq_length = target_length;
        return true;
    }

    const RawSequence* RawSequenceBook::where(size_t hash) const noexcept {
        auto iter = this->hash_table.find(hash);
        if (iter == this->hash_table.end()) {
            return nullptr;
        }
        return &(this->sequences[iter->second]);
    }

    const RawSequence *RawSequenceBook::where(const std::vector<oper_name_t> &op_str) const noexcept {
        if (op_str.size() > this->max_seq_length) {
            [[unlikely]]
            return nullptr;
        }
        return this->where(this->context.hash(op_str));
    }

}