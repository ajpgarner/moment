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

            enum class PivotStatus {
                /** Canonical link is incorrect */
                NotPivot,
                /** Canonical link is correct, and this is the first such node in the list with this status */
                Pivot,
                /** Canonical link is correct, but another node also has this status */
                FalsePivot
            } pivot_status = PivotStatus::NotPivot;

            /**
             * Request to move a link (change its origin)
             * @param it_link The link to be altered (from 'base' to 'pivot')
             * @param can_link The link from canonical to pivot
             * @param rtb The relationship between this node
             */
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

        explicit SymbolNodeSimplifyImpl(SymbolTree& tree) : theTree(tree) { }

        void simplify();

    private:
        void simplifyNode(size_t node_id);

        /**
         * Search for nodes pointed to by base_node that already have a canonical origin (i.e. have already been
         *  discovered during another part of the algorithm).
         * @param base_node The node, whose children we will consider
         * @param rebase_list The output list of discovered nodes already with a canonical origin
         * @return The index in rebase_list corresponding to the pivot node (i.e. node with lowest canonical origin)
         */
        size_t find_already_linked(SymbolTree::SymbolNode * base_node, std::vector<RebaseInfoImpl>& rebase_list);

        SymbolTree::SymbolLink * rebaseNodes(SymbolTree::SymbolNode *base_node,
                                             std::vector<RebaseInfoImpl> &rebase_list,
                                             size_t lowest_node_found_index);

        void incorporate_all_descendents(SymbolTree::SymbolNode * base_node,
                                         SymbolTree::SymbolNode * rebase_node,
                                         EqualityType base_et);


        /** Further rearrange network, such that node clusters that evaluate to zero are explicitly aliased as zero. */
        void sweep_zero();

        /** Check that nullity (re/im = 0) of nodes is shared between parents and children */
        void propagate_nullity();

        /** Count the number of nodes that have a canonical origin, and are hence aliases of other symbols */
        size_t count_noncanonical_nodes();
    };
}