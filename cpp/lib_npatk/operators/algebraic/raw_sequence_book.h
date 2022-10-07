/**
 * raw_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "raw_sequence.h"
#include "symbolic/symbol.h"

#include <map>
#include <vector>

namespace NPATK {

    class Context;

    class RawSequenceBook {
    private:
        const Context& context;
        size_t max_seq_length = 0;
        std::vector<RawSequence> sequences;
        std::vector<Symbol> symbols;

        std::map<size_t, size_t> hash_table;

    public:
        explicit RawSequenceBook(const Context& context);

        /**
         * Generate all permutations of symbols up to nominated length.
         * @param length The string length to generate sequences up to.
         * @return True, if new symbols were generated.
         */
        bool generate(size_t length);

        [[nodiscard]] size_t longest_sequence() const noexcept { return this->max_seq_length; }

        [[nodiscard]] auto begin() const noexcept { return this->sequences.cbegin(); }

        [[nodiscard]] auto end() const noexcept { return this->sequences.cend(); }

        [[nodiscard]] size_t size() const noexcept { return this->sequences.size(); }

        [[nodiscard]] const RawSequence& operator[](size_t index) const noexcept { return this->sequences[index]; }

        [[nodiscard]] const RawSequence * where(size_t hash) const noexcept;

        [[nodiscard]] const RawSequence * where(const std::vector<oper_name_t>& op_str) const noexcept;

        [[nodiscard]] const std::vector<Symbol>& Symbols() const noexcept { return this->symbols; }

    };

}