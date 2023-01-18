/**
 * enumerate_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "enumerate_symbols.h"

#include "symbolic/symbol_set.h"

#include "matrix/moment_matrix.h"

#include "import/read_symbol_or_fail.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"

#include "MatlabEngine/engine_interface_util.hpp"

namespace Moment::mex {
    namespace {

        struct FindSymbolsInSymmetricMatrix {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = SymbolSet;

            explicit FindSymbolsInSymmetricMatrix(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    // Diagonal elements are always real
                    SymbolExpression diag_elem{static_cast<symbol_name_t>(matrix[index_i][index_i])};
                    symbols_found.add_or_merge(Symbol{diag_elem.id, false});

                    // Look at off-diagonal elements
                    size_t initial_j = index_i + 1;
                    for (size_t index_j = initial_j; index_j < matrix_dimension; ++index_j) {

                        SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                        SymbolExpression txElem{static_cast<symbol_name_t>(matrix[index_j][index_i])};

                        if (elem.id != txElem.id) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be symmetric.");
                        }

                        // Casting from int type, will never be conjugated - but might be negated
                        if (elem.negated() != txElem.negated()) {
                            // a = -a; and hence symbol is zero. Should have been omitted!
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be symmetric.");
                        }

                        // Otherwise, symbol on off diagonal could be complex.
                        symbols_found.add_or_merge(Symbol{elem.id, true});
                    }
                }
                return symbols_found;
            }

            return_type string(const matlab::data::StringArray &matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    // Diagonal elements are always real
                    SymbolExpression diag_elem{read_symbol_or_fail(engine, matrix, index_i, index_i)};
                    symbols_found.add_or_merge(Symbol{diag_elem.id, false});

                    // Look at off-diagonal elements
                    size_t initial_j = index_i + 1;
                    for (size_t index_j = initial_j; index_j < matrix_dimension; ++index_j) {

                        SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                        SymbolExpression txElem{read_symbol_or_fail(engine, matrix, index_j, index_i)};

                        if (elem.id != txElem.id) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be symmetric.");
                        }

                        // Now, have to account for potential conjugation and negation of off-diagonal symbols
                        bool hasReal = true;
                        bool hasImaginary = true;

                        if (elem.negated() != txElem.negated()) {
                            if (elem.conjugated == txElem.conjugated) {
                                // a = -a, or a* = -a* => a= -a; symbol must be zero.
                                throw_error(this->engine, errors::internal_error,
                                            "Element found that cannot be symmetric.");
                            } else {
                                // a = -a*, a* = -a, -a = a* or -a* = a; symbol must be pure imaginary.
                                hasReal = false;
                            }
                        } else if (elem.conjugated != txElem.conjugated) {
                            // a = a*, -a = -a*, a* = a, -a* = -a; symbol must be real.
                            hasImaginary = false;
                        }

                        // Otherwise, symbol on off-diagonal could be real, complex or imaginary!
                        symbols_found.add_or_merge(Symbol{elem.id, !hasReal, !hasImaginary});
                    }
                }
                return symbols_found;
            }

            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {

                // Get random-access version of sparse array
                auto sparse_array_copy = sparse_array_to_map<data_t, symbol_name_t>(matrix);

                SymbolSet symbols_found{};

                for (const auto& [indices, value] : sparse_array_copy ) {
                    const auto abs_value = (value >= 0 ? value : -value);

                    // Lower diagonal - ignore, except to check that element is matched!
                    if (indices.second < indices.first) {
                        if (!sparse_array_copy.contains(std::make_pair(indices.second, indices.first))) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be symmetric.");
                        }
                        continue;
                    }

                    // Diagonal, register as real--only symbol
                    if (indices.first == indices.second) {
                        symbols_found.add_or_merge(Symbol{abs_value, false}); // Register purely real diagonal element.
                        continue;
                    }

                    auto txIter = sparse_array_copy.find(std::make_pair(indices.second, indices.first));
                    if (txIter == sparse_array_copy.end()) {
                        throw_error(this->engine, errors::internal_error,
                                    "Element found that cannot be symmetric.");
                    }
                    const auto& txValue = txIter->second;

                    // Since no conjugate values can be specified in this sparse matrix, values MUST match exactly.
                    if (value != txValue) {
                        throw_error(this->engine, errors::internal_error,
                                    "Element found that cannot be symmetric.");
                    }

                    // Add potentially complex element
                    symbols_found.add_or_merge(Symbol{abs_value, true});
                }

                return symbols_found;
            }
        };

        static_assert(concepts::VisitorHasRealDense<FindSymbolsInSymmetricMatrix>);
        static_assert(concepts::VisitorHasRealSparse<FindSymbolsInSymmetricMatrix>);
        static_assert(concepts::VisitorHasString<FindSymbolsInSymmetricMatrix>);

        struct FindSymbolsInHermitianMatrix {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = SymbolSet;

            explicit FindSymbolsInHermitianMatrix(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    // Diagonal elements are always real
                    SymbolExpression diag_elem{static_cast<symbol_name_t>(matrix[index_i][index_i])};
                    symbols_found.add_or_merge(Symbol{diag_elem.id, false});

                    // Look at off-diagonal elements
                    size_t initial_j = index_i + 1;
                    for (size_t index_j = initial_j; index_j < matrix_dimension; ++index_j) {

                        SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                        SymbolExpression txElem{static_cast<symbol_name_t>(matrix[index_j][index_i])};

                        if (elem.id != txElem.id) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be Hermitian.");
                        }

                        // Casting from int type, will never be conjugated - but might be negated
                        if (elem.negated() != txElem.negated()) {
                            // a* = -a or -a* = a; and hence symbol is purely imaginary
                            symbols_found.add_or_merge(Symbol{elem.id, true, false});
                        } else {
                            // Otherwise, symbol on off-diagonal is real (because no conjugation!)
                            symbols_found.add_or_merge(Symbol{elem.id, false, true});
                        }
                    }
                }
                return symbols_found;
            }


            return_type string(const matlab::data::StringArray &matrix) {
                size_t matrix_dimension = matrix.getDimensions()[0];
                SymbolSet symbols_found{};

                // Iterate over upper portion
                for (size_t index_i = 0; index_i < matrix_dimension; ++index_i) {
                    // Diagonal elements are always real
                    SymbolExpression diag_elem{read_symbol_or_fail(engine, matrix, index_i, index_i)};
                    symbols_found.add_or_merge(Symbol{diag_elem.id, false});

                    // Look at off-diagonal elements
                    size_t initial_j = index_i + 1;
                    for (size_t index_j = initial_j; index_j < matrix_dimension; ++index_j) {

                        SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                        SymbolExpression txElem{read_symbol_or_fail(engine, matrix, index_j, index_i)};
                        txElem.conjugated = !txElem.conjugated; // So we compare...

                        if (elem.id != txElem.id) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be Hermitian.");
                        }

                        // Now, have to account for potential conjugation and negation of off-diagonal symbols
                        bool hasReal = true;
                        bool hasImaginary = true;

                        if (elem.negated() != txElem.negated()) {
                            if (elem.conjugated == txElem.conjugated) {
                                // a = -a, or a* = -a* => a= -a; symbol must be zero.
                                throw_error(this->engine, errors::internal_error,
                                            "Element found that cannot be Hermitian.");
                            } else {
                                // a = -a*, a* = -a, -a = a* or -a* = a; symbol must be pure imaginary.
                                hasReal = false;
                            }
                        } else if (elem.conjugated != txElem.conjugated) {
                            // a = a*, -a = -a*, a* = a, -a* = -a; symbol must be real.
                            hasImaginary = false;
                        }

                        // Otherwise, symbol on off-diagonal could be real, complex or imaginary!
                        symbols_found.add_or_merge(Symbol{elem.id, !hasReal, !hasImaginary});
                    }
                }
                return symbols_found;
            }


            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {

                // Get random-access version of sparse array
                auto sparse_array_copy = sparse_array_to_map<data_t, symbol_name_t>(matrix);
                SymbolSet symbols_found{};

                for (const auto& [indices, value] : sparse_array_copy ) {
                    const auto abs_value = (value >= 0 ? value : -value);

                    // Lower diagonal - ignore, except to check that element is matched!
                    if (indices.second < indices.first) {
                        if (!sparse_array_copy.contains(std::make_pair(indices.second, indices.first))) {
                            throw_error(this->engine, errors::internal_error,
                                        "Element found that cannot be symmetric.");
                        }
                        continue;
                    }

                    // Diagonal, register as real--only symbol
                    if (indices.first == indices.second) {
                        symbols_found.add_or_merge(Symbol{abs_value, false}); // Register purely real diagonal element.
                        continue;
                    }

                    auto txIter = sparse_array_copy.find(std::make_pair(indices.second, indices.first));
                    if (txIter == sparse_array_copy.end()) {
                        throw_error(this->engine, errors::internal_error,
                                    "Element found that cannot be Hermitian.");
                    }
                    const auto& txValue = txIter->second;

                    // Since no conjugate values can be specified in this sparse matrix, values MUST match exactly.
                    if (value == txValue) {
                        // Matching sign, could be real
                        symbols_found.add_or_merge(Symbol{abs_value, false, true});
                    } else if (value == -txValue) {
                        // Mismatched sign, must be imaginary (or zero)
                        symbols_found.add_or_merge(Symbol{abs_value, true, false});
                    } else {
                        throw_error(this->engine, errors::internal_error,
                                    "Element found that cannot be symmetric.");
                    }
                }

                return symbols_found;
            }

        };

        static_assert(concepts::VisitorHasRealDense<FindSymbolsInHermitianMatrix>);
        static_assert(concepts::VisitorHasRealSparse<FindSymbolsInHermitianMatrix>);
        static_assert(concepts::VisitorHasString<FindSymbolsInHermitianMatrix>);

        SymbolSet enumerate_symmetric_symbols(matlab::engine::MATLABEngine &engine, const matlab::data::Array &matrix) {
            SymbolSet symbols_found{DispatchVisitor(engine, matrix, FindSymbolsInSymmetricMatrix{engine})};
            return symbols_found;
        }

        SymbolSet enumerate_hermitian_symbols(matlab::engine::MATLABEngine &engine, const matlab::data::Array &matrix) {
            SymbolSet symbols_found{DispatchVisitor(engine, matrix, FindSymbolsInHermitianMatrix{engine})};
            return symbols_found;
        }
    }

    MatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                            const matlab::data::Array& matrix,
                                            MatrixType basis_type) {

        // Get symbols in matrix...
        SymbolSet symbols_found{(basis_type == MatrixType::Symmetric)
                                ? enumerate_symmetric_symbols(engine, matrix)
                                : enumerate_hermitian_symbols(engine, matrix)};

        // Get matrix dimensions
        size_t matrix_dimension = matrix.getDimensions()[0];

        // Make IMP object
        return MatrixProperties{matrix_dimension, basis_type, symbols_found};
    }

}