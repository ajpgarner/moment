/**
 * symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"
#include "symbolic/symbol.h"
#include "symbolic/symbol_expression.h"

#include <cassert>
#include <map>
#include <set>
#include <vector>

namespace NPATK {

    class UniqueSequence {
        symbol_name_t id = -1;
        OperatorSequence opSeq;
        std::optional<OperatorSequence> conjSeq{};
        size_t fwd_hash = 0;
        size_t conj_hash = 0;
        bool hermitian = false;

    public:
        constexpr UniqueSequence(OperatorSequence sequence, size_t hash) :
                opSeq{std::move(sequence)}, fwd_hash{hash},
                conjSeq{}, conj_hash{hash},
                hermitian{true} { }

        constexpr UniqueSequence(OperatorSequence sequence, size_t hash,
                                 OperatorSequence conjSequence, size_t conjHash) :
                opSeq{std::move(sequence)}, fwd_hash{hash},
                conjSeq{std::move(conjSequence)}, conj_hash{conjHash},
                hermitian{false} { }

        [[nodiscard]] constexpr symbol_name_t Id() const noexcept { return this->id; }
        [[nodiscard]] constexpr size_t hash() const noexcept { return this->fwd_hash; }
        [[nodiscard]] constexpr size_t hash_conj() const noexcept { return this->conj_hash; }
        [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept { return this->opSeq; }
        [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
            return this->hermitian ? opSeq : this->conjSeq.value();
        }
        /**
         * Does the operator sequence represent its Hermitian conjugate?
         * If true, the element will correspond to a real symbol (cf. complex if not) in the NPA matrix.
         */
        [[nodiscard]] constexpr bool is_hermitian() const noexcept { return this->hermitian; }

        inline static UniqueSequence Zero(const Context& context) {
            return UniqueSequence{OperatorSequence::Zero(&context), 0};
        }

        inline static UniqueSequence Identity(const Context& context) {
            return UniqueSequence{OperatorSequence::Identity(&context), 1};
        }

        friend class SymbolTable;
    };


    class SymbolTable {
    private:
        /** Context, for simplifying operator sequences */
        const Context& context;

        /** Unique sequence list */
        std::vector<UniqueSequence> unique_sequences{};

        /** Maps hash to unique symbol; +ve is forward element, -ve is Hermitian conjugate.
         * Invariant promise: non-hermitian elements will have both forward and reverse hashes saved. */
        std::map<size_t, ptrdiff_t> hash_table;

    public:
        SymbolTable(const Context& context);

        SymbolTable(SymbolTable&& rhs) noexcept;



        /**
         * Prune list of hashes, removing elements already in the Symbol Table.
         * @param build_unique Map of hashes to sequences.
         * @return Pair, First: Map of hashes to sequences, without elements already in the SymbolTable.
         *              Second: symbol ids of duplicate elements removed.
         */
        std::pair<std::map<size_t, UniqueSequence>, std::set<symbol_name_t>>
        remove_duplicates(std::map<size_t, UniqueSequence>&& build_unique);

        std::set<symbol_name_t> merge_in(std::map<size_t, UniqueSequence>&& build_unique);

        [[nodiscard]] auto begin() const noexcept { return this->unique_sequences.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->unique_sequences.cend(); }
        [[nodiscard]] bool empty() const noexcept { return this->unique_sequences.empty(); }
        [[nodiscard]] size_t size() const noexcept { return this->unique_sequences.size(); }

       /**
         * Get the unique sequence at a supplied index.
         */
        [[nodiscard]] const UniqueSequence& operator[](size_t index) const noexcept {
            assert(index < this->unique_sequences.size());
            return this->unique_sequences[index];
        }

        /**
        * Find the unique sequence matching supplied operator string.
        * @param seq The sequence to match
        * @return Pointer to unique sequence element if matched, nullptr otherwise.
        */
        [[nodiscard]] const UniqueSequence * where(const OperatorSequence& seq) const noexcept;

        /**
         * Find symbol expression matching supplied operator sequence.
         * @param seq The sequence to match
         * @return The SymbolExpression matching the sequence, or zero if not found.
         */
        [[nodiscard]] SymbolExpression to_symbol(const OperatorSequence& seq) const noexcept;

        /**
         * Find ID, and conjugation status, of element in unique_sequences corresponding to hash.
         * ID return will be numeric-limit max of size_t if no element found.
         * @param hash The hash to look up
         * @return Pair: First gives the element in unique_sequences, second is true if hash corresponds to conjugate.
         */
        [[nodiscard]] std::pair<size_t, bool> hash_to_index(size_t hash) const noexcept;
    };

}