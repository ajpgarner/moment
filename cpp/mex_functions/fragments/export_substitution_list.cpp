/**
 * export_substitution_list.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_substitution_list.h"
#include "../utilities/reporting.h"

namespace NPATK::mex {
    matlab::data::Array export_substitution_list(matlab::engine::MATLABEngine& engine,
                                                       const SymbolTree& tree) {
        using namespace matlab::engine;
        assert(tree.ready());
        matlab::data::ArrayFactory factory{};

        // Nothing to do if nothing to substitute
        if (tree.alias_count() <= 0) {
            return factory.createEmptyArray();
        }

        // Otherwise, we create a ML-string array of aliases:
        matlab::data::ArrayDimensions dims{tree.alias_count(), 2};
        auto sub_list = factory.createArray<matlab::data::MATLABString>(dims);

        size_t write_cursor = 0;
        const size_t node_count = tree.count_nodes();
        for (size_t node_id = 1; node_id < node_count; ++node_id) {
            const auto& node = tree[node_id];
            if (node.unaliased()) {
                continue;
            }
            assert(write_cursor < tree.alias_count());

            sub_list[write_cursor][0] = convertUTF8StringToUTF16String(SymbolExpression{node.id}.as_string());
            sub_list[write_cursor][1] = convertUTF8StringToUTF16String(node.canonical_expression().as_string());

            ++write_cursor;
        }
        return sub_list;
    }
}