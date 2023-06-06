/**
 * substituted_matrix_properties.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_properties.h"

#include <string>

namespace Moment {

    class SubstitutedMatrixProperties : public MatrixProperties {
    public:
        SubstitutedMatrixProperties(MatrixProperties&& source, std::string new_desc) noexcept
            : MatrixProperties{std::move(source)} {
            this->set_description(std::move(new_desc));
        }
    };
}