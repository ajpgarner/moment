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

    SymbolSet::SymbolSet()
            : Symbols{*this}, Links{*this} {
        this->add_or_merge(Symbol::zero());
    }

    SymbolSet::SymbolSet(const std::vector<Symbol> &in_symbols)
            : Symbols{*this}, Links{*this} {
        this->add_or_merge(Symbol::zero());

        for (const auto& symbol : in_symbols) {
            this->add_or_merge(symbol);
        }
    }

    SymbolSet::SymbolSet(const std::vector<SymbolPair>& raw_pairs)
        : Symbols{*this}, Links{*this} {
        this->add_or_merge(Symbol::zero());

        for (const auto& rule : raw_pairs) {
            this->add_or_merge(rule);
        }
    }

    SymbolSet::SymbolSet(const std::vector<Symbol> &extra_symbols, const std::vector<SymbolPair> &raw_pairs)
        : Symbols{*this}, Links{*this} {
        this->add_or_merge(Symbol::zero());

        for (const auto& symbol : extra_symbols) {
            this->add_or_merge(symbol);
        }

        for (const auto& rule : raw_pairs) {
            this->add_or_merge(rule);
        }
    }


    SymbolSet::SymbolSet(const SymbolTree &tree)
        : Symbols{*this}, Links{*this},
          packing_key{tree.packing_map},
          unpacking_key{tree.unpacking_map},
          packed{true} {

        for (const auto& node : tree.tree_nodes) {
            // Copy node into symbol map
            symbols.emplace_hint(symbols.end(), std::make_pair(node.id, static_cast<Symbol>(node)));
            // If node is not fundamental, add link:
            if (!node.unaliased()) {
                auto rule = node.canonical_pair();
                equality_map_t::key_type key{rule.left_id, rule.right_id};
                EqualityType eq_type = equality_type(rule);
                this->symbol_links.insert({key, eq_type});
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

    bool SymbolSet::add_or_merge(const SymbolPair &rule, bool force_real) {
        equality_map_t::key_type key{rule.left_id, rule.right_id};
        EqualityType eq_type = equality_type(rule);

        // Add symbol names
        this->add_or_merge(Symbol{rule.left_id, !force_real});
        this->add_or_merge(Symbol{rule.right_id, !force_real});

        // Add, or update, link:
        auto [iter, new_element] = this->symbol_links.insert({key, eq_type});
        if (!new_element) {
            iter->second = iter->second | eq_type;
            return false;
        }
        return true;
    }



    void SymbolSet::reset() noexcept {
        this->symbols.clear();
        this->symbol_links.clear();
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
        if (!symbolSet.symbol_links.empty()) {
            os << "Symbol links:\n";
            for (auto [names, link_type]: symbolSet.symbol_links) {
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