/**
 * symbol_table.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "symbol_expression.h"

#include "scenarios/dictionary_map.h"
#include "scenarios/operator_sequence.h"

#include "utilities/dynamic_bitset.h"

#include <cassert>

#include <iosfwd>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

namespace Moment {

    namespace errors {
        class zero_symbol : public std::runtime_error {
        public:
            const symbol_name_t id;
            explicit zero_symbol(symbol_name_t id);
        };

        class unknown_symbol : public std::range_error {
        public:
            const symbol_name_t id;
            explicit unknown_symbol(symbol_name_t id);
        };

        class unknown_basis_elem : public std::range_error {
        public:
            const ptrdiff_t id;
            const bool real;
            unknown_basis_elem(bool is_real, ptrdiff_t id);
        };
    }

    class UniqueSequence {
    private:
        symbol_name_t id = -1;
        std::optional<OperatorSequence> opSeq{};
        std::optional<OperatorSequence> conjSeq{};
        bool hermitian = false;
        bool antihermitian = false;
        ptrdiff_t real_index = -1;
        ptrdiff_t img_index = -1;
        std::string fwd_sequence_str;

    public:
        constexpr UniqueSequence() = default;

        explicit UniqueSequence(OperatorSequence sequence) :
                opSeq{std::move(sequence)},
                conjSeq{}, hermitian{true}, antihermitian{false},
                real_index{-1}, img_index{-1}, fwd_sequence_str{opSeq->formatted_string()} { }

        UniqueSequence(OperatorSequence sequence, OperatorSequence conjSequence);

        /** True if a concrete operator sequence is associated with this symbol */
        [[nodiscard]] constexpr bool has_sequence() const noexcept { return this->opSeq.has_value(); }

        /** The symbol ID */
        [[nodiscard]] constexpr symbol_name_t Id() const noexcept { return this->id; }

        /**
         * The hash associated with the operator sequence.
         * Undefined behaviour if no operator sequence associated with this entry.
         */

        [[nodiscard]] constexpr size_t hash() const noexcept {
            assert(this->opSeq.has_value());
            return this->opSeq.value().hash();
        }

        /**
         * The hash associated with the operator sequence's complex conjugate..
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr size_t hash_conj() const noexcept {
            assert(this->conjSeq.has_value() || this->opSeq.has_value());
            return this->conjSeq.has_value() ? this->conjSeq->hash() : this->opSeq.value().hash();
        }

        /**
         * The operator sequence associated with this entry.
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept {
            assert(this->opSeq.has_value());
            return this->opSeq.value();
        }

        /**
         * The operator sequence associated with this entry's complex conjugate.
         * Undefined behaviour if no operator sequence associated with this entry.
         */
        [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
            assert(this->conjSeq.has_value() || this->opSeq.has_value());
            return this->hermitian ? opSeq.value() : this->conjSeq.value();
        }

        /**
         * Formatted view of sequence, if any, otherwise just symbol name.
         */
        [[nodiscard]] std::string formatted_sequence() const;

        /**
         * Formatted view of conjugate sequence, if any, otherwise just symbol name.
         */
        [[nodiscard]] std::string formatted_sequence_conj() const;

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

        /**
         * Named constructor for entry associated with '0'.
         * @param context The operator context.
         */
        inline static UniqueSequence Zero(const Context& context) {
            auto us = UniqueSequence{OperatorSequence::Zero(context)};
            us.id = 0;
            us.hermitian = true;
            us.antihermitian = true;
            return us;
        }

        /**
         * Named constructor for entry associated with '1'.
         * @param context The operator context.
         */
        inline static UniqueSequence Identity(const Context& context) {
            auto us = UniqueSequence{OperatorSequence::Identity(context)};
            us.id = 1;
            us.hermitian = true;
            us.antihermitian = false;
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


    public:
        class BasisView {
        private:
            /** Back reference to containing object */
            SymbolTable& symbol_table;

            /**
              * Ordered list of symbols with real components.
              * Effectively find x such that of UniqueSequence[x].real_index == index.
              */
            std::vector<size_t> real_symbols;

            /**
             * Ordered list of symbols with imaginary components (i.e. corresponding to non-Hermitian operators)
             * Effectively find x such that of UniqueSequence[x].img_index == index
             */
            std::vector<size_t> imaginary_symbols;

            /**
             * Cross list: associated imaginary basis element with real basis element (or -1).
             */
            std::vector<ptrdiff_t> im_of_real;

            /**
             * Cross list: associated real basis element with imaginary basis element (or -1).
             */
            std::vector<ptrdiff_t> re_of_imaginary;


        private:
            explicit BasisView(SymbolTable& table) noexcept
                    : symbol_table{table} { }

            /**
             * Go through symbols, and re-number real/imaginary bases based on whether symbols can be have
             * real or imaginary parts.
             * @return A pair: the number of real basis elements, and the number of imaginary basis elements.
             */
            std::pair<size_t, size_t> renumerate_bases();


            std::pair<ptrdiff_t, ptrdiff_t> push_back(symbol_name_t symbol_id, bool has_real, bool has_imaginary);

        public:
            /**
             * Vector of symbol IDs of every symbol containing a real component.
             */
            [[nodiscard]] const auto& RealSymbols() const noexcept { return this->real_symbols; }

            /**
             * Number of symbols with real parts.
             */
            [[nodiscard]] size_t RealSymbolCount() const noexcept { return this->real_symbols.size(); }

            /**
             * Vector of symbol IDs of every symbol containing an imaginary component.
             */
            [[nodiscard]] const auto& ImaginarySymbols() const noexcept { return this->imaginary_symbols; }


            /**
             * Number of symbols with imaginary parts.
             */
            [[nodiscard]] size_t ImaginarySymbolCount() const noexcept { return this->imaginary_symbols.size(); }


            /**
             * Get basis key associated with symbol id
             */
            [[nodiscard]] std::pair<ptrdiff_t, ptrdiff_t> operator()(symbol_name_t symbol_id) const noexcept {
                assert((symbol_id >= 0) && (symbol_id < this->symbol_table.unique_sequences.size()));
                return std::make_pair(this->symbol_table.unique_sequences[symbol_id].real_index,
                                      this->symbol_table.unique_sequences[symbol_id].img_index);
            }

            /**
             * Get imaginary part associated with real basis element
             */
            [[nodiscard]] ptrdiff_t imaginary_from_real(ptrdiff_t re_basis_elem) const noexcept {
                assert(re_basis_elem <= this->im_of_real.size());
                if (re_basis_elem < 0) {
                    return -1;
                }
                return this->im_of_real[re_basis_elem];
            }

            /**
             * Get real part associated with imaginary basis element
             */
            [[nodiscard]] ptrdiff_t real_from_imaginary(ptrdiff_t im_basis_elem) const noexcept {
                assert(im_basis_elem <= this->re_of_imaginary.size());
                if (im_basis_elem < 0) {
                    return -1;
                }
                return this->re_of_imaginary[im_basis_elem];
            }


            friend class SymbolTable;
        } Basis;

    public:
        /**
         * Map from operator-sequence-generator output indices to symbol ids in this table.
         */
        DictionaryMap OSGIndex;

    public:
        /**
         * Constructs a symbol table, associated with a particular context.
         * @param context The context, for simplifying and formatting operator sequences.
         */
        explicit SymbolTable(const Context& context);

        /**
         * Add symbols to table, if not already present
         * @param build_unique List of symbols to be potentially merged
         * @paran new_symbols Output: number of new symbols if not nullptr.
         * @return Set of symbol IDs
         */
        std::set<symbol_name_t> merge_in(std::vector<UniqueSequence>&& build_unique, size_t * new_symbols = nullptr);

        /**
         * Add symbol to table, if not already present by OperatorSequence
         * @param build_unique Symbols to be potentially merge
         * @return The ID of the (possibly new) symbol.
         */
        symbol_name_t merge_in(OperatorSequence&& sequence);

        /**
         * Add symbol to table, if not already present
         * @param build_unique Symbols to be potentially merge
         * @return The ID of the (possibly new) symbol.
         */
        symbol_name_t merge_in(UniqueSequence&& sequence);

        /**
         * Add symbols to table, if not already present, and adjust real/imaginary zeros of those already present
         * @param can_be_real Bit set of symbols with real parts
         * @param can_be_imaginary Bit set of symbols with imaginary parts
         * @return True, if existing symbol realness was modified
         */
        bool merge_in(const DynamicBitset<uint64_t>& can_be_real, const DynamicBitset<uint64_t>& can_be_imaginary);

        /**
         * Add empty symbol to table.
         * @param has_real True if symbol has real parts
         * @param has_imaginary True if symbol has imaginary parts
         * @param name The string the symbol represents.
         * @return The ID of the new symbol.
         */
        symbol_name_t create(bool has_real = true, bool has_imaginary = true, std::string name = "");

        /**
         * Add multiple empty symbols to table.
         * @return The ID of the first new symbol.
         */
        symbol_name_t create(size_t count, bool has_real = true, bool has_imaginary = true);


        /**
         * Generate all symbols up to a particular word length (merging with existing symbols).
         * For thread safety, a write lock should be called on the owning matrix system first.
         * @return Pair, first: number of symbols up to that word length, second: number of new symbols added.
         */
         std::pair<size_t, size_t> fill_to_word_length(size_t word_length);

        /**
         * Begin iteration over symbols of the table
         */
        [[nodiscard]] auto begin() const noexcept { return this->unique_sequences.cbegin(); }

        /**
         * End iteration over symbols of the table
         */
        [[nodiscard]] auto end() const noexcept { return this->unique_sequences.cend(); }

        /**
         * True if no symbols in table
         */
        [[nodiscard]] bool empty() const noexcept { return this->unique_sequences.empty(); }

        /**
         * Number of symbols in table
         */
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
         * Find the unique sequence matching supplied operator string, and if it is conjugated.
         * @param seq The sequence to match
         * @return First, pointer to unique sequence element if matched, nullptr otherwise. Second true if conjugated.
         */
        [[nodiscard]] std::pair<const UniqueSequence *, bool>
        where_and_is_conjugated(const OperatorSequence &seq) const noexcept;

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
        [[nodiscard]] std::pair<ptrdiff_t, bool> hash_to_index(size_t hash) const noexcept;

        /**
         * Output symbol table, as debug info
         */
         friend std::ostream& operator<<(std::ostream& os, const SymbolTable& table);
    };

}
