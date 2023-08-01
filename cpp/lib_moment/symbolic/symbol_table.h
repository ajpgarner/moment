/**
 * symbol_table.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "symbol.h"
#include "symbol_lookup_result.h"
#include "monomial.h"

#include "dictionary/dictionary_map.h"
#include "dictionary/operator_sequence.h"

#include "utilities/dynamic_bitset_fwd.h"

#include <cassert>

#include <iosfwd>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
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


    /**
     * List of symbols associated with matrix system.
     */
    class SymbolTable {
    private:
        /** Context, for simplifying operator sequences */
        const Context& context;

        /** Unique sequence list */
        std::vector<Symbol> unique_sequences{};

        /** Maps hash to unique symbol; +ve is forward element, -ve is Hermitian conjugate.
         * Invariant promise: non-hermitian elements will have both forward and reverse hashes saved. */
        std::map<size_t, ptrdiff_t> hash_table;

    public:
        /** True if aliased symbols could be present (that is, two operator sequences mapping to same moment). */
        const bool can_have_aliases = false;

    public:
        class BasisView {
        private:
            /** Back reference to containing object */
            SymbolTable& symbol_table;

            /**
              * Ordered list of symbols with real components.
              * Effectively find x such that of Symbol[x].real_index == index.
              */
            std::vector<size_t> real_symbols;

            /**
             * Ordered list of symbols with imaginary components (i.e. corresponding to non-Hermitian operators)
             * Effectively find x such that of Symbol[x].img_index == index
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
        std::set<symbol_name_t> merge_in(std::vector<Symbol>&& build_unique, size_t * new_symbols = nullptr);

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
        inline symbol_name_t merge_in(Symbol&& elem, size_t * const new_symbols = nullptr) {
            symbol_name_t symbol_id;
            std::tie(symbol_id, std::ignore) = this->merge_in_with_hash_hint(this->hash_table.begin(),
                                                                             std::move(elem), new_symbols);
            return symbol_id;
        }

        /**
         * Add symbols to table, if not already present.
         * @param iter Start of range of symbols to add.
         * @param iter_end End of range of symbols to add.
         * @return The IDs of the (possibly new) symbol.
         */
        std::set<symbol_name_t> merge_in(std::map<size_t, Symbol>::iterator iter,
                                         std::map<size_t, Symbol>::iterator iter_end,
                                         size_t * added = nullptr);

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
        [[nodiscard]] const Symbol& operator[](size_t index) const noexcept {
            assert(index < this->unique_sequences.size());
            return this->unique_sequences[index];
        }

        /**
        * Find the unique sequence matching supplied operator string.
        * @param seq The sequence to match
        * @return Pointer to unique sequence element if matched, nullptr otherwise.
        */
        [[nodiscard]] SymbolLookupResult where(const OperatorSequence& seq) const noexcept;

        /**
         * Find symbol expression matching supplied operator sequence.
         * @param seq The sequence to match
         * @return The Monomial matching the sequence, or zero if not found.
         */
        [[nodiscard]] Monomial to_symbol(const OperatorSequence& seq) const noexcept;


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

    private:
        /**
        * Add symbol to table, if not already present
        * @param hint Iterator to hash value not after hash of input symbol.
        * @param sequence Symbol to be potentially merged.
        * @return The ID of the (possibly new) symbol, and iterator to its position in the hash.
        */
        std::pair<symbol_name_t, std::map<size_t, ptrdiff_t>::iterator>
        merge_in_with_hash_hint(std::map<size_t, ptrdiff_t>::iterator hint,
                                Symbol&& sequence,
                                size_t * new_symbols = nullptr);
    };

}
