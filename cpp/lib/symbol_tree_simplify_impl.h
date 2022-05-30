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
        SymbolTree& theTree;

    public:

        SymbolNodeSimplifyImpl(SymbolTree& tree) : theTree(tree) { }

        void simplify();

    private:
        void simplifyNode(SymbolTree& theTree, size_t node_id);

        size_t find_already_linked(SymbolTree::SymbolNode * base_node, std::vector<RebaseInfoImpl>& rebase_list);

        void incorporate_all_descendents(SymbolTree::SymbolNode * base_node,
                                         SymbolTree::SymbolNode * rebase_node,
                                         EqualityType base_et);

        SymbolTree::SymbolLink * rebaseNodes(SymbolTree::SymbolNode *this_node,
                                             std::vector<RebaseInfoImpl> &rebase_list,
                                             size_t lowest_node_found_index);

        void sweep_zero();

        void propagate_nullity();
    };

}