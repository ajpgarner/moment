/**
 * enumerate_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "enumerate_symbols.h"
#include "symbol_set.h"

#include "reporting.h"
#include "MatlabDataArray/ArrayVisitors.hpp"

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

            template<typename generic>
            SymbolSet operator()(const generic&) {
                throw_error(engine, "Unsupported type.");
                throw; // hint
            }

            template<typename data_t>
            SymbolSet operator()(const matlab::data::TypedArray<data_t>& matrix) {
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

            template<typename data_t>
            [[noreturn]] SymbolSet operator()(const matlab::data::TypedArray<std::complex<data_t>>& matrix) {
                throw_error(engine, "Complex numbers not yet supported.");
                throw; // hint~
            }

            SymbolSet sparse(const matlab::data::SparseArray<double>& matrix) {
                SymbolSet symbols_found{};

                auto iter = matrix.cbegin();
                while (iter != matrix.cend()) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};
                    auto indices = matrix.getIndex(iter);
                    // Only over upper portion
                    if (indices.second > indices.first) {
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

    IndexMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                            const matlab::data::Array& matrix,
                                            IndexMatrixProperties::BasisType basis_type,
                                            bool debug_output) {
        // Get matrix dimensions
        size_t matrix_dimension = matrix.getDimensions()[0];

        // Get symbols in matrix...
        FindSymbols visitor{engine, basis_type};
        SymbolSet symbols_found{};
        if (matrix.getType() == matlab::data::ArrayType::SPARSE_DOUBLE) {
            symbols_found = visitor.sparse(matrix);
        } else {
            symbols_found = matlab::data::apply_numeric_visitor(matrix, visitor);
        }

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