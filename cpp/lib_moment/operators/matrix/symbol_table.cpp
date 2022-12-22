/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_table.h"

#include "../context.h"

#include <iostream>

namespace Moment {
    UniqueSequence::UniqueSequence(OperatorSequence sequence,
                                   OperatorSequence conjSequence):
            opSeq{std::move(sequence)},
            conjSeq{std::move(conjSequence)},
            hermitian{false}, antihermitian{false}, real_index{-1}, img_index{-1} {

        int compare = OperatorSequence::compare_same_negation(opSeq, *conjSeq);
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