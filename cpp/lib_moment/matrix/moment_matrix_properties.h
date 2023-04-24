/**
 * moment_matrix_properties.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_properties.h"

namespace Moment {

    class MomentMatrixProperties : public MatrixProperties {
    private:
        size_t level;
    public:
        MomentMatrixProperties(const Matrix& matrix, const SymbolTable& table, std::set<symbol_name_t>&& subset,
                                                 const std::string& description, bool is_hermitian, size_t mm_level)
            : MatrixProperties{matrix, table, std::move(subset), description, is_hermitian}, level{mm_level} { }

        MomentMatrixProperties(MatrixProperties&& rhs, size_t mm_level,
                               bool override_hermitian, std::string override_desc) noexcept
        : MatrixProperties{std::move(rhs)}, level{mm_level} {
            this->override_hermicity(override_hermitian);
            this->set_description(std::move(override_desc));
        }

        [[nodiscard]] size_t Level() const noexcept { return this->level; }
    };
}