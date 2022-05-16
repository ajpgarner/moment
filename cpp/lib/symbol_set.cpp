/**
 * symbol_set.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_set.h"
#include "symbol_tree.h"
#include <cassert>
#include <iostream>



namespace  NPATK {
    SymbolSet::SymbolSet(const std::vector<SymbolPair>& raw_pairs)
        : Symbols{*this}, Links{*this}
    {

        for (const auto& rule : raw_pairs) {
            equality_map_t::key_type key{rule.left_id, rule.right_id};
            EqualityType eq_type = equality_type(rule);

            // Add symbol names
            this->add_or_merge(Symbol{rule.left_id});
            this->add_or_merge(Symbol{rule.right_id});

            // Add, or update, link:
            auto [iter, new_element] = this->symbol_links.insert({key, eq_type});
            if (!new_element) {
                iter->second = iter->second | eq_type;
            }
        }
    }


    SymbolSet::SymbolSet(const std::vector<Symbol> &extra_symbols, const std::vector<SymbolPair> &raw_pairs)
        : Symbols{*this}, Links{*this} {

        // First, add "extra" symbols
        for (const auto& symbol : extra_symbols) {
            this->add_or_merge(symbol);
        }

        for (const auto& rule : raw_pairs) {
            equality_map_t::key_type key{rule.left_id, rule.right_id};
            EqualityType eq_type = equality_type(rule);

            // Add symbol names
            this->add_or_merge(Symbol{rule.left_id});
            this->add_or_merge(Symbol{rule.right_id});

            // Add, or update, link:
            auto [iter, new_element] = this->symbol_links.insert({key, eq_type});
            if (!new_element) {
                iter->second = iter->second | eq_type;
            }
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
        this->symbol_links.clear();
        this->packing_key.clear();
        this->unpacking_key.clear();
        this->packed = false;
    }

    std::ostream& operator<<(std::ostream &os, const SymbolSet &symbolSet) {

        for (auto [names, link_type] : symbolSet.symbol_links) {
            bool done_one = false;

            if ((link_type & EqualityType::equal) == EqualityType::equal) {
                os << names.first << " == " << names.second;
                done_one = true;
            }

            if ((link_type & EqualityType::negated) == EqualityType::negated) {
                if (done_one) {
                    os << " AND ";
                }
                os << names.first << " == -" << names.second;
                done_one = true;
            }

            if ((link_type & EqualityType::conjugated) == EqualityType::conjugated) {
                if (done_one) {
                    os << " AND ";
                }
                os << names.first << " == " << names.second << "*";
                done_one = true;
            }

            if ((link_type & EqualityType::conjugated) == EqualityType::conjugated) {
                if (done_one) {
                    os << " AND ";
                }
                os << names.first << " == -" << names.second << "*";
                done_one = true;
            }
            os << "\n";
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

        equality_map_t renamed_links{};
        for (auto [key, value] : this->symbol_links) {
            symbol_name_t source = this->packing_key[key.first];
            symbol_name_t target = this->packing_key[key.second];
            renamed_links.insert(
                {{source, target},
                 value}
             );
        }
        std::swap(this->symbol_links, renamed_links);




        this->packed = true;
    }

    void SymbolSet::unpack() {
        if (!this->packed) {
            return;
        }

        equality_map_t renamed_links{};
        for (auto [key, value] : this->symbol_links) {
            symbol_name_t source = this->unpacking_key[key.first];
            symbol_name_t target = this->unpacking_key[key.second];
            renamed_links.insert(
                    {{source, target},
                     value}
            );
        }
        std::swap(this->symbol_links, renamed_links);

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