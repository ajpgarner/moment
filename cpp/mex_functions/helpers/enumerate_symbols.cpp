/**
 * enumerate_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "enumerate_symbols.h"
#include "symbol_set.h"

#include "reporting.h"
#include "visitor.h"

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
                        if (!matrix[index_i][index_j].has_value()) {
                            std::stringstream errMsg;
                            errMsg << "Element [" << index_i << ", " << index_j << " was empty.";
                            throw_error(engine, errMsg.str());
                        }
                        try {
                            NPATK::SymbolExpression elem{matlab::engine::convertUTF16StringToUTF8String(matrix[index_i][index_j])};
                            bool could_be_complex = (basis_type == IndexMatrixProperties::BasisType::Hermitian)
                                                    && (index_i != index_j);
                            symbols_found.add_or_merge(Symbol{elem.id, could_be_complex});
                        } catch (const SymbolExpression::SymbolParseException& e) {
                            std::stringstream errMsg;
                            errMsg << "Error converting element [" << index_i << ", " << index_j << ": " << e.what();
                            throw_error(engine, errMsg.str());
                        }
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
        FindSymbols visitor{engine, basis_type};
        VisitDispatcher dispatcher{engine, visitor};
        SymbolSet symbols_found{dispatcher(matrix)};

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