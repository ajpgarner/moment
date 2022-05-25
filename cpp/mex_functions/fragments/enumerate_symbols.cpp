/**
 * enumerate_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "enumerate_symbols.h"
#include "symbol_set.h"

#include "../utilities/reporting.h"
#include "../utilities/visitor.h"
#include "read_symbol_or_fail.h"

#include "MatlabEngine/engine_interface_util.hpp"

namespace NPATK::mex {
    namespace {
        struct FindSymbols {
        private:
            matlab::engine::MATLABEngine& engine;
            IndexMatrixProperties::BasisType basis_type;

        public:
            FindSymbols(matlab::engine::MATLABEngine& engineRef,
                              IndexMatrixProperties::BasisType basisType)
                  : engine(engineRef),  basis_type(basisType) { }

            using return_type = SymbolSet;

            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t>& matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    for (size_t index_j = index_i; index_j < matrix_dimension; ++index_j) {
                        NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                        bool could_be_complex = (basis_type == IndexMatrixProperties::BasisType::Hermitian)
                                                && (index_i != index_j);
                        symbols_found.add_or_merge(Symbol{elem.id, could_be_complex});
                    }
                }
                return symbols_found;
            }

            return_type string(const matlab::data::StringArray& matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    for (size_t index_j = index_i; index_j < matrix_dimension; ++index_j) {

                        NPATK::SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                        bool could_be_complex = (basis_type == IndexMatrixProperties::BasisType::Hermitian)
                                                    && (index_i != index_j);
                        symbols_found.add_or_merge(Symbol{elem.id, could_be_complex});
                    }
                }
                return symbols_found;
            }

            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t>& matrix) {
                SymbolSet symbols_found{};

                auto iter = matrix.cbegin();
                while (iter != matrix.cend()) {


                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};
                    auto indices = matrix.getIndex(iter);
                    // Only over upper portion
                    if (indices.first > indices.second) {
                        ++iter;
                        continue;
                    }

                    bool could_be_complex = (basis_type == IndexMatrixProperties::BasisType::Hermitian)
                                            && (indices.first != indices.second);
                    symbols_found.add_or_merge(Symbol{elem.id, could_be_complex});

                    ++iter;
                }
                return symbols_found;
            }
        };
    }

    static_assert(concepts::VisitorHasRealDense<FindSymbols>);
    static_assert(concepts::VisitorHasRealSparse<FindSymbols>);
    static_assert(concepts::VisitorHasString<FindSymbols>);

    IndexMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                            const matlab::data::Array& matrix,
                                            IndexMatrixProperties::BasisType basis_type,
                                            bool debug_output) {
        // Get matrix dimensions
        size_t matrix_dimension = matrix.getDimensions()[0];

        // Get symbols in matrix...
        SymbolSet symbols_found{DispatchVisitor(engine, matrix, FindSymbols{engine, basis_type})};

        // Report symbols detected, if debug mode enabled
        if (debug_output) {
            std::stringstream ss;
            ss << "enumerate_symbols found following:\n" << symbols_found << "\n";
            print_to_console(engine, ss.str());
        }

        // Construct matrix property structure
        return IndexMatrixProperties{matrix_dimension, basis_type, std::move(symbols_found)};
    }
}