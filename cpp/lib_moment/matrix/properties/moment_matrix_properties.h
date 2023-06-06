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
        MomentMatrixProperties(MatrixProperties&& rhs, size_t mm_level,
                               bool override_hermitian, std::string override_desc) noexcept
        : MatrixProperties{std::move(rhs)}, level{mm_level} {
            this->set_hermicity(override_hermitian);
            this->set_description(std::move(override_desc));
        }

        [[nodiscard]] size_t Level() const noexcept { return this->level; }
    };
}