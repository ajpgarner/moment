/**
 * symbol_table.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbol_table.h"

#include "dictionary/operator_sequence_generator.h"
#include "scenarios/context.h"

#include "utilities/dynamic_bitset.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <tuple>

namespace Moment {

    namespace errors {
        namespace {
            std::string make_zs_err_msg(symbol_name_t id) {
                std::stringstream errMsg;
                errMsg << "Symbol " << id << " is identically zero; but zero should be uniquely represented as \"0\"";
                return errMsg.str();
            }

            std::string make_us_err_msg(symbol_name_t id) {
                std::stringstream errMsg;
                errMsg << "Symbol " << id << " is not defined in symbol table.";
                return errMsg.str();
            }

            std::string make_ube_err_msg(bool real, ptrdiff_t id) {
                std::stringstream errMsg;
                if (real) {
                    errMsg << "Real";
                } else {
                    errMsg << "Imaginary";
                }
                errMsg << " basis element " << id << " is not defined in symbol table.";
                return errMsg.str();
            }
        }

        zero_symbol::zero_symbol(symbol_name_t sid) : std::runtime_error{make_zs_err_msg(sid)}, id{sid} { }

        unknown_symbol::unknown_symbol(Moment::symbol_name_t sid) : std::range_error{make_us_err_msg(sid)}, id{sid} { }

        unknown_basis_elem::unknown_basis_elem(bool real_or, ptrdiff_t basis_id)
        : std::range_error{make_ube_err_msg(real_or, basis_id)}, real{real_or}, id{basis_id} { }
    }

    SymbolTable::SymbolTable(const Context& context)
        : Basis{*this}, context{context}, OSGIndex{context, *this},
          can_have_aliases{context.can_have_aliases()} {

        // Zero and identity are always in symbol table, in indices 0 and 1 respectively.
        this->unique_sequences.emplace_back(Symbol::Zero(this->context));
        this->unique_sequences.emplace_back(Symbol::Identity(this->context));

        this->hash_table.insert({this->unique_sequences[0].hash(), 0});
        this->hash_table.insert({this->unique_sequences[1].hash(), 1});

        // '1' is always in real basis.
        this->Basis.push_back(1, true, false);
    }



    symbol_name_t SymbolTable::create(const bool has_real, const bool has_imaginary) {
        auto next_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        Symbol blank;
        blank.id = next_id;

        auto [re_index, im_index] = this->Basis.push_back(next_id, has_real, has_imaginary);
        blank.real_index = re_index;
        blank.img_index = im_index;

        if (!has_real) {
            blank.antihermitian = true;
        }
        if (!has_imaginary) {
            blank.hermitian = true;
        }

        this->unique_sequences.emplace_back(std::move(blank));

        return next_id;
    }

    symbol_name_t SymbolTable::create(const size_t count, const bool has_real, const bool has_imaginary) {
        const auto first_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        const auto range_end = first_id + count;

        this->unique_sequences.reserve(range_end);
        if (has_real) {
            this->Basis.real_symbols.reserve(this->Basis.real_symbols.size() + count);
        }
        if (has_imaginary) {
            this->Basis.imaginary_symbols.reserve(this->Basis.imaginary_symbols.size() + count);
        }

        for (size_t next_id = first_id; next_id < range_end; ++next_id) {
            Symbol blank;
            blank.id = static_cast<symbol_name_t>(next_id);

            auto [re_index, im_index] = this->Basis.push_back(blank.id, has_real, has_imaginary);
            blank.real_index = re_index;
            blank.img_index = im_index;

            if (!has_real) {
                blank.antihermitian = true;
            }
            if (!has_imaginary) {
                blank.hermitian = true;
            }

            this->unique_sequences.emplace_back(std::move(blank));
        }
        return first_id;
    }

    std::set<symbol_name_t> SymbolTable::merge_in(std::vector<Symbol> &&build_unique, size_t * const newly_added) {
        std::set<symbol_name_t> included_symbols;
        for (auto& elem : build_unique) {
            const auto symbol_name = this->merge_in(std::move(elem), newly_added);
            included_symbols.emplace(symbol_name);
        }
        return included_symbols;
    }

    std::set<symbol_name_t>
    SymbolTable::merge_in(std::map<size_t, Symbol>::iterator iter,
                          std::map<size_t, Symbol>::iterator iter_end,
                          size_t * const new_symbols) {
        std::set<symbol_name_t> included_symbols;
        auto hash_iter = this->hash_table.begin();
        while (iter != iter_end) {

            symbol_name_t symbol_name;
            std::tie(symbol_name, hash_iter) = this->merge_in_with_hash_hint(hash_iter,
                                                                             std::move(iter->second),
                                                                             new_symbols);
            included_symbols.emplace(symbol_name);
            ++iter;
        }
        return included_symbols;
    }



    symbol_name_t SymbolTable::merge_in(OperatorSequence&& sequence) {
        // First, is sequence canonical?
        if (this->can_have_aliases) {
            auto alias = this->context.simplify_as_moment(std::move(sequence));
            auto alias_conj = alias.conjugate();
            return this->merge_in(Symbol{std::move(alias), std::move(alias_conj)});
        }

        // Otherwise, directly attempt merge
        auto conj_seq = sequence.conjugate();
        return this->merge_in(Symbol{std::move(sequence), std::move(conj_seq)});
    }



    std::pair<symbol_name_t, std::map<size_t, ptrdiff_t>::iterator>
    SymbolTable::merge_in_with_hash_hint(std::map<size_t, ptrdiff_t>::iterator hint,
                                         Symbol&& elem, size_t * new_symbols) {
        // Look for insertion point in hash table (with hints)
        auto find_hash = std::lower_bound(hint, this->hash_table.end(), elem.hash(),
                                          [](const auto& lhs, const auto& value) { return lhs.first < value; });

        // Not unique, do not add
        if ((find_hash != this->hash_table.end()) && find_hash->first == elem.hash()) {
            const ptrdiff_t stIndex = find_hash->second >= 0 ? find_hash->second : -find_hash->second;
            return {this->unique_sequences[stIndex].id, find_hash};
        }

        // Error if attempting to add an aliased symbol.
        assert(!this->can_have_aliases || !elem.has_sequence()
               || !this->context.can_be_simplified_as_moment(elem.sequence()));

        // Otherwise, query
        const auto next_index = static_cast<symbol_name_t>(this->unique_sequences.size());

        // Does context know about nullity?
        auto [re_zero, im_zero] = this->context.is_sequence_null(elem.sequence());

        // Is element hermitian
        const bool is_hermitian = elem.is_hermitian();
        if (is_hermitian) {
            im_zero = true;
        }

        // Is element anti-hermitian
        const bool is_anti_hermitian = elem.is_antihermitian();
        if (is_anti_hermitian) {
            re_zero = true;
        }

        // Make element
        elem.id = next_index;

        // Add to basis
        const auto [re_index, im_index] = this->Basis.push_back(next_index, !re_zero, !im_zero);
        elem.real_index = re_index;
        elem.img_index = im_index;

        // Prepare output
        std::pair<symbol_name_t, std::map<size_t, ptrdiff_t>::iterator> output;
        output.first = next_index;

        // Add hash(es)
        output.second = this->hash_table.emplace_hint(find_hash, std::make_pair(elem.hash(), next_index));
        if (!is_hermitian) {
            this->hash_table.emplace(std::make_pair(elem.hash_conj(), -next_index));
        }

        // Register element
        this->unique_sequences.emplace_back(std::move(elem));

        // Flag as added
        if (new_symbols != nullptr) {
            ++(*new_symbols);
        }

        return output;
    }



    bool SymbolTable::merge_in(const DynamicBitset<uint64_t>& can_be_real,
                               const DynamicBitset<uint64_t>& can_be_imaginary) {
        assert(can_be_real.bit_size == can_be_imaginary.bit_size);
        bool changes = false;

        // Ensure enough elements exist
        const size_t initial_elems = this->unique_sequences.size();
        const size_t elems = can_be_real.bit_size;
        if (elems > initial_elems) {
            this->create(elems - initial_elems);
        }

        // Go through symbols, flagging where they must be real / imaginary
        for (auto& symbol : this->unique_sequences) {
            if (0 == symbol.id) {
                symbol.hermitian = true;
                symbol.antihermitian = true;
                continue;
            }

            const bool sym_has_real = can_be_real.test(symbol.id);
            const bool sym_has_imaginary = can_be_imaginary.test(symbol.id);
            if (sym_has_real) {
                if (!sym_has_imaginary) {
                    if (!symbol.hermitian) {
                        changes = true;
                    }
                    symbol.hermitian = true;
                    if (symbol.antihermitian) {
                        throw errors::zero_symbol{symbol.id};
                    }
                }
            } else if (sym_has_imaginary) {
                if (!symbol.antihermitian) {
                    changes = true;
                }
                symbol.antihermitian = true;
                if (symbol.hermitian) {
                    throw errors::zero_symbol{symbol.id};
                }
            } else {
                throw errors::zero_symbol{symbol.id};
            }
        }

        // With new real/imaginary information, re-count the bases
        this->Basis.renumerate_bases();

        return changes;
    }


    SymbolLookupResult
    SymbolTable::where(const OperatorSequence &seq) const noexcept {
        const size_t hash = this->context.hash(seq);
        auto [id, conj] = this->hash_to_index(hash);
        // Found normal.
        if (id != std::numeric_limits<ptrdiff_t>::max()) {
            assert(id < this->unique_sequences.size());
            return {&this->unique_sequences[id], conj, false};
        }

        // Try aliases
        if (this->can_have_aliases) {
            auto aliasedSeq = this->context.simplify_as_moment(OperatorSequence(seq));
            const size_t alias_hash = aliasedSeq.hash();
            if (alias_hash != seq.hash()) {
                auto [alias_id, alias_conj] = this->hash_to_index(alias_hash);
                // Found alias?
                if (alias_id != std::numeric_limits<ptrdiff_t>::max()) {
                    assert(alias_id < this->unique_sequences.size());
                    return {&this->unique_sequences[alias_id], alias_conj, true};
                } else {
                    return {nullptr, false, true}; // Not found, but also not canonical.
                }
            }
        }

        // Not found
        return {};
    }


    Monomial SymbolTable::to_symbol(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);
        auto [id, conj] = this->hash_to_index(hash);
        if (id == std::numeric_limits<ptrdiff_t>::max()) {
            return Monomial{0};
        }

        return Monomial{this->unique_sequences[id].id, conj};
    }


    std::pair<ptrdiff_t, bool> SymbolTable::hash_to_index(size_t hash) const noexcept {
        auto hash_iter = this->hash_table.find(hash);
        if (hash_iter == this->hash_table.end()) {
            return {std::numeric_limits<ptrdiff_t>::max(), false};
        }

        if (hash_iter->second >= 0) {
            return {hash_iter->second, false};
        } else {
            return {-hash_iter->second, true};
        }
    }

    std::pair<size_t, size_t> SymbolTable::fill_to_word_length(size_t word_length) {

        const auto& osg = this->context.operator_sequence_generator(word_length);

        // Handle case where we have already generated a longer symbol table.
        if (this->OSGIndex.max_length() > word_length) {
            return std::make_pair(osg.size(), 0);
        }

        // Merge in symbols and update OSGIndex
        std::map<size_t, Symbol> build_unique;
        for (const auto& op_seq : osg) {
            // Skip aliased symbols
            if (this->can_have_aliases && this->context.can_be_simplified_as_moment(op_seq)) {
                continue;
            }

            auto conj_seq = op_seq.conjugate();
            const size_t seq_hash = op_seq.hash();
            const size_t conj_hash = conj_seq.hash();

            if (seq_hash == conj_hash) {
                build_unique.emplace_hint(build_unique.end(),
                                          std::make_pair(seq_hash, Symbol{op_seq}));
            } else if (seq_hash < conj_hash) {
                build_unique.emplace_hint(build_unique.end(),
                                          std::make_pair(seq_hash, Symbol{op_seq, std::move(conj_seq)}));
            }
        }
        size_t new_symbols{};
        this->merge_in(build_unique.begin(), build_unique.end(), &new_symbols);

        this->OSGIndex.update(word_length);

        return std::make_pair(osg.size(), new_symbols);
    }


    std::pair<size_t, size_t> SymbolTable::BasisView::renumerate_bases() {
        ptrdiff_t real_index = 0;
        ptrdiff_t imaginary_index = 0;
        this->real_symbols.clear();
        this->imaginary_symbols.clear();
        this->im_of_real.clear();
        this->re_of_imaginary.clear();

        for (auto& symbol : symbol_table.unique_sequences) {
            const bool has_real = !symbol.antihermitian;
            const bool has_im = !symbol.hermitian;

            if (has_real) {
                this->real_symbols.emplace_back(symbol.id);
                symbol.real_index = real_index;
                ++real_index;
            } else {
                symbol.real_index = -1;
            }

            if (has_im) {
                this->imaginary_symbols.emplace_back(symbol.id);
                symbol.img_index = imaginary_index;
                this->re_of_imaginary.emplace_back(symbol.real_index);
                ++imaginary_index;
            } else {
                symbol.img_index = -1;
            }

            if (has_real) {
                this->im_of_real.emplace_back(symbol.img_index);
            }
        }
        return {real_index, imaginary_index};
    }


    std::pair<ptrdiff_t, ptrdiff_t> SymbolTable::BasisView::push_back(const Moment::symbol_name_t symbol_id,
                                                                      const bool has_real, const bool has_im) {
        std::pair<ptrdiff_t, ptrdiff_t> output{-1, -1};
        if (has_real) {
            output.first = static_cast<ptrdiff_t>(this->real_symbols.size());
            this->real_symbols.emplace_back(symbol_id);
        }
        if (has_im) {
            output.second = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
            this->imaginary_symbols.emplace_back(symbol_id);
            this->re_of_imaginary.emplace_back(output.first);
        }
        if (has_real) {
            this->im_of_real.emplace_back(output.second);
        }
        return output;
    }


    std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {
        const auto& real_symbols = table.Basis.RealSymbols();
        const auto& im_symbols = table.Basis.ImaginarySymbols();

        os << "Symbol table with ";
        os << table.unique_sequences.size()
           << " unique sequence" << ((table.unique_sequences.size() != 1)  ? "s" : "") << ", ";
        os << real_symbols.size() << " with real parts, "
           << im_symbols.size() << " with imaginary parts:\n";

        // List real symbol IDs
        if (real_symbols.empty()) {
            os << "No symbols with real parts.\n";
        } else {
            os << "Symbols with real parts: ";
            bool one_real = false;
            for (auto i: real_symbols) {
                if (one_real) {
                    os << ", ";
                }
                os << i;
                one_real = true;
            }
            os << "\n";
        }

        // List imaginary symbol IDs
        if (im_symbols.empty()) {
            os << "No symbols with imaginary parts.\n";
        } else {
            os << "Symbols with imaginary parts: ";
            bool one_im = false;
            for (auto i: im_symbols) {
                if (one_im) {
                    os << ", ";
                }
                os << i;
                one_im = true;
            }
            os << "\n";
        }

        // List symbols
        for (const auto& us : table.unique_sequences) {
            os << us << "\n";
        }

        return os;
    }

}