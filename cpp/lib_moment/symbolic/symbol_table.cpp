/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#include "symbol_table.h"

#include "scenarios/context.h"

#include "utilities/dynamic_bitset.h"


#include <iostream>
#include <sstream>

namespace Moment {

    namespace errors {
        std::string make_zs_err_msg(symbol_name_t id) {
            std::stringstream errMsg;
            errMsg << "Symbol " << id << " is identically zero; but zero should be uniquely represented as \"0\"";
            return errMsg.str();
        }

        errors::zero_symbol::zero_symbol(symbol_name_t sid) : std::runtime_error{make_zs_err_msg(sid)}, id{sid} { }
    }

    UniqueSequence::UniqueSequence(OperatorSequence sequence,
                                   OperatorSequence conjSequence):
            opSeq{std::move(sequence)},
            conjSeq{std::move(conjSequence)},
            hermitian{false}, antihermitian{false}, real_index{-1}, img_index{-1} {

        int compare = OperatorSequence::compare_same_negation(*opSeq, *conjSeq);
        if (1 == compare) {
            this->hermitian = true;
        } else if (-1 == compare) {
            this->antihermitian = true;
        }
    }

    SymbolTable::SymbolTable(const Context& context)
        : context{context} {

        // Zero and identity are always in symbol table, in indices 0 and 1 respectively.
        this->unique_sequences.emplace_back(UniqueSequence::Zero(this->context));
        this->unique_sequences.emplace_back(UniqueSequence::Identity(this->context));

        this->hash_table.insert({this->unique_sequences[0].hash(), 0});
        this->hash_table.insert({this->unique_sequences[1].hash(), 1});
        this->real_symbols.emplace_back(1);
    }


    std::set<symbol_name_t> SymbolTable::merge_in(std::vector<UniqueSequence> &&build_unique) {
        std::set<symbol_name_t> included_symbols;

        for (auto& elem : build_unique) {
            const auto symbol_name = this->merge_in(std::move(elem));
            included_symbols.emplace(symbol_name);
        }

        return included_symbols;
    }

    symbol_name_t SymbolTable::merge_in(UniqueSequence&& elem) {
        // Not unique, do not add...
        auto existing_iter = this->hash_table.find(elem.hash());
        if (existing_iter != this->hash_table.end()) {
            const ptrdiff_t stIndex = existing_iter->second >= 0 ? existing_iter->second : -existing_iter->second;
            return this->unique_sequences[stIndex].id;
        }

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

        // Real part
        if (!re_zero) {
            elem.real_index = static_cast<ptrdiff_t>(this->real_symbols.size());
            this->real_symbols.emplace_back(next_index);
        } else {
            elem.real_index = -1;
        }

        // Imaginary part
        if (!im_zero) {
            elem.img_index = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
            this->imaginary_symbols.emplace_back(next_index);
        } else {
            elem.img_index = -1;
        }

        // Add hash(es)
        this->hash_table.emplace(std::make_pair(elem.hash(), next_index));
        if (!is_hermitian) {
            this->hash_table.emplace(std::make_pair(elem.hash_conj(), -next_index));
        }

        // Register element
        this->unique_sequences.emplace_back(std::move(elem));

        return next_index;
    }



    symbol_name_t SymbolTable::create(const bool has_real, const bool has_imaginary) {
        auto next_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        UniqueSequence blank;
        blank.id = next_id;
        if (has_real) {
            blank.real_index = static_cast<ptrdiff_t>(this->real_symbols.size());
            this->real_symbols.push_back(blank.id);
        }
        if (has_imaginary) {
            blank.img_index = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
            this->imaginary_symbols.push_back(blank.id);
        }
        this->unique_sequences.emplace_back(std::move(blank));

        return next_id;
    }

    symbol_name_t SymbolTable::create(const size_t count, const bool has_real, const bool has_imaginary) {
        const auto first_id = static_cast<symbol_name_t>(this->unique_sequences.size());
        const auto range_end = first_id + count;

        this->unique_sequences.reserve(range_end);
        if (has_real) {
            this->real_symbols.reserve(this->real_symbols.size() + count);
        }
        if (has_imaginary) {
            this->imaginary_symbols.reserve(this->imaginary_symbols.size() + count);
        }

        for (size_t next_id = first_id; next_id < range_end; ++next_id) {
            UniqueSequence blank;
            blank.id = static_cast<symbol_name_t>(next_id);
            if (has_real) {
                blank.real_index = static_cast<ptrdiff_t>(this->real_symbols.size());
                this->real_symbols.push_back(blank.id);
            }
            if (has_imaginary) {
                blank.img_index = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
                this->imaginary_symbols.push_back(blank.id);
            }
            this->unique_sequences.emplace_back(std::move(blank));
       }
        return first_id;
    }

    void SymbolTable::merge_in(const DynamicBitset<uint64_t>& can_be_real,
                               const DynamicBitset<uint64_t>& can_be_imaginary) {
        assert(can_be_real.bit_size == can_be_imaginary.bit_size);

        // Ensure enough elements exist
        const size_t elems = can_be_real.bit_size;
        if (elems > this->unique_sequences.size()) {
            this->create(elems - this->unique_sequences.size());
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
                    symbol.hermitian = true;
                    if (symbol.antihermitian) {
                        throw errors::zero_symbol{symbol.id};
                    }
                }
            } else if (sym_has_imaginary) {
                symbol.antihermitian = true;
                if (symbol.hermitian) {
                    throw errors::zero_symbol{symbol.id};
                }
            } else {
                throw errors::zero_symbol{symbol.id};
            }
        }

        // With new real/imaginary information, re-count the bases
        this->renumerate_bases();
    }


    std::pair<size_t, size_t> SymbolTable::renumerate_bases() {
        ptrdiff_t real_index = 0;
        ptrdiff_t imaginary_index = 0;
        this->real_symbols.clear();
        this->imaginary_symbols.clear();
        for (auto& symbol : this->unique_sequences) {
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
                ++imaginary_index;
            } else {
                symbol.img_index = -1;
            }
        }
        return {real_index, imaginary_index};
    }

    const UniqueSequence *
    SymbolTable::where(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);

        auto [id, conj] = this->hash_to_index(hash);
        if (id == std::numeric_limits<ptrdiff_t>::max()) {
            return nullptr;
        }

        assert(id < this->unique_sequences.size());
        return &this->unique_sequences[id];
    }

    SymbolExpression SymbolTable::to_symbol(const OperatorSequence &seq) const noexcept {
        size_t hash = this->context.hash(seq);
        auto [id, conj] = this->hash_to_index(hash);
        if (id == std::numeric_limits<ptrdiff_t>::max()) {
            return SymbolExpression{0};
        }

        return SymbolExpression{this->unique_sequences[id].id, conj};
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

    std::ostream& operator<<(std::ostream& os, const UniqueSequence& seq) {
        os << "#" << seq.id << ":\t";
        os << seq.sequence() << ":\t";

        if (seq.real_index>=0) {
            if (seq.img_index>=0) {
                os << "Complex";
            } else {
                os << "Real";
            }
        } else if (seq.img_index>=0) {
            os << "Imaginary";
        } else {
            os << "Zero";
        }

        if (seq.hermitian) {
            os << ", Hermitian";
        }
        if (seq.real_index>=0) {
            os << ", Re#=" << seq.real_index;
        }
        if (seq.img_index>=0) {
            os << ", Im#=" << seq.img_index;
        }
        os << ", hash=" << seq.hash();
        if (seq.hash_conj() != seq.hash()) {
            os << "/" << seq.hash_conj();
        }
        return os;
    }

    std::string UniqueSequence::formatted_sequence() const {
        if (this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "S" << this->id;
        return ss.str();
    }

    std::string UniqueSequence::formatted_sequence_conj() const {
        if (this->conjSeq.has_value()) {
            return this->opSeq->formatted_string();
        } else if (this->hermitian && this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "S" << this->id << "*";
        return ss.str();
    }

    std::ostream& operator<<(std::ostream& os, const SymbolTable& table) {
        os << "Symbol table with ";
        os << table.unique_sequences.size()
           << " unique sequence" << ((table.unique_sequences.size() != 1)  ? "s" : "") << ", ";
        os << table.real_symbols.size() << " with real parts, "
           << table.imaginary_symbols.size() << " with imaginary parts:\n";

        // List real symbol IDs
        if (table.real_symbols.empty()) {
            os << "No symbols with real parts.\n";
        } else {
            os << "Symbols with real parts: ";
            bool one_real = false;
            for (auto i: table.real_symbols) {
                if (one_real) {
                    os << ", ";
                }
                os << i;
                one_real = true;
            }
            os << "\n";
        }

        // List imaginary symbol IDs
        if (table.imaginary_symbols.empty()) {
            os << "No symbols with imaginary parts.\n";
        } else {
            os << "Symbols with imaginary parts: ";
            bool one_im = false;
            for (auto i: table.imaginary_symbols) {
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