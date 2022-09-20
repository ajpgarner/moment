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

    std::pair<std::map<size_t, UniqueSequence>, std::set<symbol_name_t>>
    SymbolTable::remove_duplicates(std::map<size_t, UniqueSequence> &&build_unique) {
        auto readIter = build_unique.begin();
        auto hashIter = hash_table.begin();

        std::pair<std::map<size_t, UniqueSequence>, std::set<symbol_name_t>> output;

        while (hashIter != hash_table.end() && readIter != build_unique.end()) {
            const size_t currentHash = readIter->first;

            // Hash table is lower than next insertion, no clash yet.
            if (hashIter->first < currentHash) {
                ++hashIter;
                continue;
            }

            // If symbol already exists, delete from 'build_unique' list.
            if (readIter->first == currentHash) {
                output.second.insert(this->unique_sequences[hashIter->second].Id());
                readIter = build_unique.erase(readIter);
                continue;
            }

            // Move to next input symbol
            ++readIter;
        }

        output.first = std::move(build_unique);
        return output;
    }


    std::set<symbol_name_t> SymbolTable::merge_in(std::map<size_t, UniqueSequence> &&input_map) {

        // First, remove symbols already in the hash table
        auto [build_unique, symbol_ids] = this->remove_duplicates(std::move(input_map));

        // Reserve space in vector
        this->unique_sequences.reserve(this->unique_sequences.size() + build_unique.size());

        // Push new elements to list
        std::map<size_t, ptrdiff_t> fwd_hash_list;
        std::map<size_t, ptrdiff_t> rev_hash_list;
        auto elem_index = static_cast<symbol_name_t>(this->unique_sequences.size());
        for (auto [hash, elem] : build_unique) {
            fwd_hash_list.insert({hash, static_cast<ptrdiff_t>(elem_index)});
            if (!elem.is_hermitian()) {
                rev_hash_list.insert({elem.conj_hash, -static_cast<ptrdiff_t>(elem_index)});
            }
            elem.id = elem_index;
            elem.real_index = static_cast<ptrdiff_t>(this->real_symbols.size());
            elem.img_index = elem.is_hermitian() ? -1 : static_cast<ptrdiff_t>(this->imaginary_symbols.size());
            this->real_symbols.emplace_back(elem_index);

            if (!elem.is_hermitian()) {
                this->imaginary_symbols.emplace_back(elem_index);
            }
            this->unique_sequences.emplace_back(std::move(elem));

            symbol_ids.insert(symbol_ids.end(), elem_index);

            ++elem_index;
        }

        assert(elem_index == this->unique_sequences.size());

        // Now, merge in hashes
        this->hash_table.merge(std::move(fwd_hash_list));
        this->hash_table.merge(std::move(rev_hash_list));

        // Return complete list of symbols in the input map
        return symbol_ids;
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


    std::pair<size_t, bool> SymbolTable::hash_to_index(size_t hash) const noexcept {
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