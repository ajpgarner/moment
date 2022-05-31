/**
 * index_matrix_properties.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"

#include <vector>
#include <map>

namespace NPATK {
    class SymbolSet;

    class IndexMatrixProperties {
    public:
        enum class MatrixType {
            Unknown = 0,
            /** Real-valued, matrix is symmetric */
            Symmetric = 1,
            /** Complex-valued, matrix is hermitian */
            Hermitian = 2
        };

    private:
        MatrixType basis_type = MatrixType::Unknown;
        size_t dimension = 0;
        std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> elem_keys{};
        std::vector<symbol_name_t> real_entries;
        std::vector<symbol_name_t> imaginary_entries;

    public:
        IndexMatrixProperties(size_t dim, MatrixType type, SymbolSet&& entries);

        [[nodiscard]] constexpr MatrixType Type() const noexcept {
            return this->basis_type;
        }

        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

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


    };
}