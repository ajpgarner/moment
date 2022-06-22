/**
 * identify_nonhermitian_elements.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "identify_nonhermitian_elements.h"

#include "fragments/read_symbol_or_fail.h"
#include "utilities/make_sparse_matrix.h"
#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"


namespace NPATK::mex {


    namespace {
        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
         */
        class NonhermitianElementIdentifierVisitor {
            matlab::engine::MATLABEngine& engine;

        public:
            using return_type = NPATK::SymbolSet;

        public:
            explicit NonhermitianElementIdentifierVisitor(matlab::engine::MATLABEngine &the_engine)
                    : engine(the_engine) { }

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            return_type string(const matlab::data::StringArray &data) {
                SymbolSet output{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Register diagonal element as real symbol:
                    NPATK::SymbolExpression diag{read_symbol_or_fail(this->engine, data, i, i)};
                    output.add_or_merge(Symbol{diag.id, false});

                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{read_symbol_or_fail(this->engine, data, i, j)};
                        NPATK::SymbolExpression lower{read_symbol_or_fail(this->engine, data, j, i)};
                        lower.conjugated = !lower.conjugated;

                        if (upper != lower) {
                            output.add_or_merge(SymbolPair{upper, lower});
                        } else {
                            output.add_or_merge(Symbol{upper.id});
                            output.add_or_merge(Symbol{lower.id});
                        }
                    }
                }

                return output;
            }
        };

        static_assert(concepts::VisitorHasString<NonhermitianElementIdentifierVisitor>);

        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
         */
        class IsHermitianVisitor {
            matlab::engine::MATLABEngine& engine;

        public:
            using return_type = bool;

        public:
            explicit IsHermitianVisitor(matlab::engine::MATLABEngine &the_engine)
                    : engine(the_engine) { }

            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                SymbolSet output{};

                std::vector<SymbolPair> non_matching{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Register diagonal element as real symbol:
                    NPATK::SymbolExpression diag{static_cast<NPATK::symbol_name_t>(data[i][i])};
                    output.add_or_merge(Symbol{diag.id, false});

                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{static_cast<NPATK::symbol_name_t>(data[i][j])};
                        NPATK::SymbolExpression lower{static_cast<NPATK::symbol_name_t>(data[j][i])};

                        // Wrong ID, can never be Hermitian
                        if (upper.id != lower.id) {
                            return false;
                        }

                        // Conjugation never specified, since symbols come from real array.
                        // If symbols negated, can be Hermitian if pure imaginary;
                        //  if not negated, can be Hermitian if pure real.
                    }
                }

                return true;
            }


            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            return_type string(const matlab::data::StringArray &data) {
                SymbolSet output{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Diagonal elements are automatically Hermitian (if real).

                    // Check off diagonal elements
                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{read_symbol_or_fail(this->engine, data, i, j)};
                        NPATK::SymbolExpression lower{read_symbol_or_fail(this->engine, data, j, i)};

                        // Never Hermitian if IDs don't match
                        if (upper.id != lower.id) {
                            return false;
                        }

                        // a = (-a*)* -> a = -a = 0; but not written as zero.
                        if ((lower.conjugated != upper.conjugated) && (lower.negated != upper.negated)) {
                            return false;
                        }

                        // Otherwise:
                        //  If a* = a, could be Hermitian if symbol is real
                        //  If a* = -a, could be Hermitian if symbol is imaginary
                    }
                }

                // No non-matching elements found, so matrix is Hermitian
                return true;
            }


            template<std::convertible_to<symbol_name_t> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {

                // Get random-access version of sparse array
                auto sparse_array_copy = sparse_array_to_map<data_t, symbol_name_t>(matrix);

                for (const auto& [indices, value] : sparse_array_copy ) {
                    const auto abs_value = (value >= 0 ? value : -value);

                    // Lower diagonal - ignore, except to check that element is matched!
                    if (indices.second < indices.first) {
                        if (!sparse_array_copy.contains(std::make_pair(indices.second, indices.first))) {
                            return false; // Unmatched symbol
                        }
                        continue;
                    }

                    // Diagonal, noting to do
                    if (indices.first == indices.second) {
                        continue;
                    }

                    auto txIter = sparse_array_copy.find(std::make_pair(indices.second, indices.first));
                    if (txIter == sparse_array_copy.end()) {
                        return false; // Unmatched symbol
                    }
                    const auto& txValue = txIter->second;

                    // Since no conjugate values can be specified in this sparse matrix, values MUST match exactly.
                    if ((value == txValue) || (value == -txValue)) {
                        // Matching sign, could be real
                        // Negated sign, could be imaginary
                        continue;
                    } else {
                        // Mismatched IDs: cannot be Hermitian
                        return false;
                    }
                }

                return true;
            }
        };

        static_assert(concepts::VisitorHasString<IsHermitianVisitor>);
        static_assert(concepts::VisitorHasRealDense<IsHermitianVisitor>);
        static_assert(concepts::VisitorHasRealSparse<IsHermitianVisitor>);
    }

    SymbolSet identify_nonhermitian_elements(matlab::engine::MATLABEngine &engine,
                                             const matlab::data::Array &data) {
        return DispatchVisitor(engine, data, NonhermitianElementIdentifierVisitor{engine});
    }

    bool is_hermitian(matlab::engine::MATLABEngine &engine, const matlab::data::Array &data) {
        return DispatchVisitor(engine, data, IsHermitianVisitor{engine});
    }
}
