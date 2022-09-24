/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_table.h"

#include "../context.h"

namespace NPATK {

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

        auto elem_index = static_cast<symbol_name_t>(this->unique_sequences.size());
        for (auto& elem : build_unique) {

            // Not unique, do not add...
            auto existing_iter = this->hash_table.find(elem.fwd_hash);
            if (existing_iter != this->hash_table.end()) {
                const ptrdiff_t stIndex = existing_iter->second >= 0 ? existing_iter->second : -existing_iter->second;
                included_symbols.emplace(this->unique_sequences[stIndex].id);
                continue;
            }

            // Add hashes
            const bool is_hermitian = elem.is_hermitian();
            elem.id = elem_index;
            elem.real_index = static_cast<ptrdiff_t>(this->real_symbols.size());
            this->hash_table.emplace(std::make_pair(elem.fwd_hash, elem_index));
            this->real_symbols.emplace_back(elem_index);
            if (is_hermitian) {
                elem.img_index = -1;
            } else {
                elem.img_index = static_cast<ptrdiff_t>(this->imaginary_symbols.size());
                this->hash_table.emplace(std::make_pair(elem.conj_hash, -elem_index));
                this->imaginary_symbols.emplace_back(elem_index);
            }
            included_symbols.emplace(elem_index);
            this->unique_sequences.emplace_back(std::move(elem));
            ++elem_index;
        }

        return included_symbols;
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



}