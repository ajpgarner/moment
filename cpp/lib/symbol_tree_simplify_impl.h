/**
 * symbol_tree_simplify_impl.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "symbol_tree.h"

namespace NPATK::detail {
    class SymbolNodeSimplifyImpl {
    private:
        struct RebaseInfoImpl {
        public:
            SymbolTree::SymbolLink * linkToMove;
            SymbolTree::SymbolLink * linkFromCanonicalNode;

            EqualityType relationToBase;
            EqualityType relationToCanonical = EqualityType::none;

            bool is_pivot = false;

            RebaseInfoImpl(SymbolTree::SymbolLink * it_link,
                           SymbolTree::SymbolLink * can_link,
                           EqualityType rtb) noexcept :
                    linkToMove(it_link),
                    linkFromCanonicalNode(can_link),
                    relationToBase(rtb) { }
        };


    private:
        SymbolTree::SymbolNode &this_node;

    public:
        explicit SymbolNodeSimplifyImpl(SymbolTree::SymbolNode& theNode) noexcept
            : this_node(theNode) { }

        void simplify();

    private:
        size_t find_already_linked(std::vector<RebaseInfoImpl>& rebase_list);

        void incorporate_all_descendents();
    };
}