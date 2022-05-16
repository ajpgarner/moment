/**
 * substitute_elements_using_tree.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "substitute_elements_using_tree.h"
#include "reporting.h"


namespace NPATK::mex {

    template<typename data_t>
    matlab::data::Array substitute_elements_using_tree_dense(matlab::data::TypedArray<data_t>&& the_array,
                                                             const SymbolTree& tree) {

        for (auto & elem : the_array) {
            NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(elem)};
            NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
            elem = static_cast<data_t>(new_symbol.id) * (new_symbol.negated ? -1 : 1);
        }

        return the_array;
    }


    matlab::data::Array substitute_elements_using_tree_sparse(matlab::data::SparseArray<double>&& the_array,
                                                              const SymbolTree& tree) {

        for (auto & elem : the_array) {
            NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(elem)};
            NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
            elem = static_cast<double>(new_symbol.id) * (new_symbol.negated ? -1 : 1);
        }
        return the_array;
    }

    matlab::data::Array substitute_elements_using_tree(matlab::engine::MATLABEngine& engine,
                                        matlab::data::Array&& the_array,
                                        const SymbolTree& tree) {
        switch(the_array.getType()) {
            case matlab::data::ArrayType::SINGLE:
                return substitute_elements_using_tree_dense<float>(std::move(the_array), tree);
            case matlab::data::ArrayType::DOUBLE:
                return substitute_elements_using_tree_dense<double>(std::move(the_array), tree);
            case matlab::data::ArrayType::INT8:
                return substitute_elements_using_tree_dense<int8_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::UINT8:
                return substitute_elements_using_tree_dense<uint8_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::INT16:
                return substitute_elements_using_tree_dense<int16_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::UINT16:
                return substitute_elements_using_tree_dense<uint16_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::INT32:
                return substitute_elements_using_tree_dense<int32_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::UINT32:
                return substitute_elements_using_tree_dense<uint32_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::INT64:
                return substitute_elements_using_tree_dense<int64_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::UINT64:
                return substitute_elements_using_tree_dense<uint64_t>(std::move(the_array), tree);
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                return substitute_elements_using_tree_sparse(std::move(the_array), tree);
            default:
                break;
        }

        NPATK::mex::throw_error(engine, "Matrix type not supported (should be matrix of real numbers).");
        throw; // never executed; hint to compiler that might not be aware previous function throws via matlab!
    }
}
