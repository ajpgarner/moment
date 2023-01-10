/**
 * symbol_set.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_set.h"
#include <cassert>
#include <iostream>

namespace Moment {

    SymbolSet::SymbolSet()  {
        this->add_or_merge(Symbol::zero());
    }

    SymbolSet::SymbolSet(const std::vector<Symbol> &in_symbols) {
        this->add_or_merge(Symbol::zero());

        for (const auto& symbol : in_symbols) {
            this->add_or_merge(symbol);
        }
    }

    bool SymbolSet::add_or_merge(const Symbol &symbol) {
        auto [ins_iter, inserted] = this->symbols.insert({symbol.id, symbol});
        if (inserted) {
            return true;
        }

        ins_iter->second.merge_in(symbol);
        return false;
    }

    void SymbolSet::reset() noexcept {
        this->symbols.clear();
        this->packing_key.clear();
        this->unpacking_key.clear();
        this->packed = false;
    }

    std::ostream& operator<<(std::ostream &os, const SymbolSet &symbolSet) {

        if (!symbolSet.symbols.empty()) {
            os << "Symbols:\n";
            for (const auto& name : symbolSet.symbols) {
                os << name.second << "\n";
            }
        }

        return os;
    }

    void SymbolSet::pack() {
        if (this->packed) {
            return;
        }

        assert(this->packing_key.empty());
        assert(this->unpacking_key.empty());

        this->unpacking_key.reserve(this->symbol_count());

        symbol_name_t elem_num = 0;
        symbol_map_t renamed_symbols;
        for (auto [symbol_id, symbol] : this->symbols) {
            this->unpacking_key.push_back(symbol_id);
            this->packing_key.insert({symbol_id, elem_num});
            symbol.id = elem_num;
            renamed_symbols.insert(renamed_symbols.end(), {elem_num, symbol});
            ++elem_num;
        }
        std::swap(this->symbols, renamed_symbols);

        this->packed = true;
    }

    void SymbolSet::unpack() {
        if (!this->packed) {
            return;
        }

        symbol_map_t renamed_symbols;
        for (auto [symbol_id, symbol] : this->symbols) {
            symbol.id = this->unpacking_key[symbol_id];
            renamed_symbols.insert(renamed_symbols.end(), {symbol.id, symbol});
        }
        std::swap(this->symbols, renamed_symbols);

        this->packing_key.clear();
        this->unpacking_key.clear();
        this->packed = false;

    }


}