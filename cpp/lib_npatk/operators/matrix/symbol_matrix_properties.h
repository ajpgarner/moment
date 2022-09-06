/**
 * symbol_matrix_properties.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol_expression.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace NPATK {

    class OperatorMatrix;
    class SymbolTable;
    class SymbolSet;

    /** Matrix type */
    enum class MatrixType {
        Unknown = 0,
        /** Real-valued, matrix is symmetric */
        Symmetric = 1,
        /** Complex-valued, matrix is hermitian */
        Hermitian = 2
    };

    /** Information about the particular symbol matrix (relative to the collection) */
    class SymbolMatrixProperties {
    private:
        size_t dimension;
        std::set<symbol_name_t> included_symbols;
        std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> elem_keys;
        std::vector<symbol_name_t> real_entries;
        std::vector<symbol_name_t> imaginary_entries;
        MatrixType basis_type = MatrixType::Unknown;


    public:
        /** Construct symbolic properties from operator matrix */
        SymbolMatrixProperties(const OperatorMatrix& matrix,
                               const SymbolTable& table,
                               std::set<symbol_name_t>&& subset);

        /** Construct symbolic properties manually (e.g. loaded via matlab array) */
        SymbolMatrixProperties(size_t dim, MatrixType type, const SymbolSet& entries);

        [[nodiscard]] constexpr const auto& RealSymbols() const noexcept {
            return this->real_entries;
        }

        [[nodiscard]] constexpr const auto& ImaginarySymbols() const noexcept {
            return this->imaginary_entries;
        }

        [[nodiscard]] constexpr const auto& BasisMap() const noexcept {
            return this->elem_keys;
        }

        [[nodiscard]] std::pair<ptrdiff_t, ptrdiff_t> BasisKey(symbol_name_t id) const {
            auto iter = this->elem_keys.find(id);
            if (iter == this->elem_keys.cend()) {
                return {-1, -1};
            }
            return iter->second;
        }

        [[nodiscard]] constexpr MatrixType Type() const noexcept {
            return this->basis_type;
        }

        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        friend class OperatorMatrix;
    };
}