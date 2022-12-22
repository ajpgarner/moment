/**
 * symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"
#include "symbol_expression.h"

#include "scenarios/operator_sequence.h"

#include <cassert>

#include <iosfwd>
#include <limits>
#include <map>
#include <set>
#include <vector>

namespace Moment {

    class UniqueSequence {
    private:
        symbol_name_t id = -1;
        OperatorSequence opSeq;
        std::optional<OperatorSequence> conjSeq{};
        bool hermitian = false;
        bool antihermitian = false;
        ptrdiff_t real_index = -1;
        ptrdiff_t img_index = -1;

    public:
        explicit constexpr UniqueSequence(OperatorSequence sequence) :
                opSeq{std::move(sequence)},
                conjSeq{}, hermitian{true}, antihermitian{false},
                real_index{-1}, img_index{-1}  { }

        UniqueSequence(OperatorSequence sequence, OperatorSequence conjSequence);

        [[nodiscard]] constexpr symbol_name_t Id() const noexcept { return this->id; }

        [[nodiscard]] constexpr size_t hash() const noexcept { return this->opSeq.hash(); }

        [[nodiscard]] constexpr size_t hash_conj() const noexcept {
            return this->conjSeq.has_value() ? this->conjSeq->hash() : this->opSeq.hash();
        }

        [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept { return this->opSeq; }

        [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
            return this->hermitian ? opSeq : this->conjSeq.value();
        }

        /**
         * Does the operator sequence represent its Hermitian conjugate?
         * If true, the element will correspond to a real symbol (cf. complex if not) in the NPA matrix.
         */
        [[nodiscard]] constexpr bool is_hermitian() const noexcept { return this->hermitian; }

        /**
         * Does the operator sequence represent its Hermitian conjugate up to a minus sign.
         */
        [[nodiscard]] constexpr bool is_antihermitian() const noexcept { return this->antihermitian; }

        /**
         * The real and imaginary offsets of this symbol in the basis (or -1, if no such offset).
         * @return Pair, first corresponding to real, second corresponding to imaginary.
         */
        [[nodiscard]] constexpr std::pair<ptrdiff_t, ptrdiff_t> basis_key() const noexcept {
            return {this->real_index, this->img_index};
        }

        inline static UniqueSequence Zero(const Context& context) {
            auto us = UniqueSequence{OperatorSequence::Zero(context)};
            us.id = 0;
            return us;
        }

        inline static UniqueSequence Identity(const Context& context) {
            auto us = UniqueSequence{OperatorSequence::Identity(context)};
            us.id = 1;
            us.real_index = 0;
            return us;
        }

        /**
         * Output unique sequence entry, as debug info
         */
        friend std::ostream& operator<<(std::ostream& os, const UniqueSequence& seq);

        friend class SymbolTable;
    };

    /**
     * List of sumbols associated with matrix system.
     */
    class SymbolTable {
    private:
        /** Context, for simplifying operator sequences */
        const Context& context;

        /** Unique sequence list */
        std::vector<UniqueSequence> unique_sequences{};

        /** Maps hash to unique symbol; +ve is forward element, -ve is Hermitian conjugate.
         * Invariant promise: non-hermitian elements will have both forward and reverse hashes saved. */
        std::map<size_t, ptrdiff_t> hash_table;

        /**
         * Ordered list of symbols with real components.
         * For now, this should just be 0, 1, ... sizeof(unique_sequences)-1.
         */
        std::vector<size_t> real_symbols;

        /**
         * Ordered list of symbols with imaginary components (i.e. corresponding to non-Hermitian operators)
         */
        std::vector<size_t> imaginary_symbols;

    public:
        /**
         * Constructs a symbol table, associated with a particular context.
         * @param context The context, for simplifying and formatting operator sequences.
         */
        explicit SymbolTable(const Context& context);

        /**
         * Move construct a symbol table.
         */
        SymbolTable(SymbolTable&& rhs) noexcept = default;

        /**
         * Vector of symbol IDs of every symbol containing a real component.
         */
        [[nodiscard]] const auto& RealSymbolIds() const noexcept { return this->real_symbols; }

        /**
         * Vector of symbol IDs of every symbol containing an imaginary component.
         */
        [[nodiscard]] const auto& ImaginarySymbolIds() const noexcept { return this->imaginary_symbols; }


        /**
         * Add symbols to table, if not already present
         * @param build_unique List of symbols to be potentially merged
         * @return Set of symbol IDs
         */
        std::set<symbol_name_t> merge_in(std::vector<UniqueSequence>&& build_unique);

        /**
         * Add symbol to table, if not already present
         * @param build_unique Symbols to be potentially merge
         * @return The ID of the (possibly new) symbol.
         */
        symbol_name_t merge_in(UniqueSequence&& sequence);


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
         * Get basis key associated with symbol id
         */
        [[nodiscard]] std::pair<ptrdiff_t, ptrdiff_t> to_basis(symbol_name_t symbol_id) const noexcept {
            assert((symbol_id >= 0) && (symbol_id < this->unique_sequences.size()));
            return std::make_pair(this->unique_sequences[symbol_id].real_index,
                                  this->unique_sequences[symbol_id].img_index);
        }

        /**
         * Find ID, and conjugation status, of element in unique_sequences corresponding to hash.
         * ID return will be numeric-limit max of size_t if no element found.
         * @param hash The hash to look up
         * @return Pair: First gives the element in unique_sequences, second is true if hash corresponds to conjugate.
         */
        [[nodiscard]] std::pair<ptrdiff_t, bool> hash_to_index(size_t hash) const noexcept;

        /**
         * Output symbol table, as debug info
         */
         friend std::ostream& operator<<(std::ostream& os, const SymbolTable& table);
    };

}
