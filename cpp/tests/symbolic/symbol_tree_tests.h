/**
 * symbol_tree_tests.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "gtest/gtest.h"

#include "symbolic/symbol_tree.h"

#include <memory>

namespace NPATK::Tests {

    //SymbolTree createTree(std::initializer_list<SymbolPair> pairs);

    class SymbolTreeFixture : public ::testing::Test {
    protected:
        std::unique_ptr<SymbolSet> source_set;
        std::unique_ptr<SymbolTree> the_tree;

        SymbolTree& create_tree(std::initializer_list<Symbol> symbols,
                                std::initializer_list<SymbolPair> pairs);

        SymbolTree& create_tree(std::initializer_list<SymbolPair> pairs);

        void compare_to(std::initializer_list<SymbolPair> pairs,
                        bool only_topology = false);

        void compare_to(std::initializer_list<Symbol> extra,
                        std::initializer_list<SymbolPair> pairs,
                        bool only_topology = false);

        void compare_to(const SymbolTree& compare_tree, bool only_topology = false);

    };

}