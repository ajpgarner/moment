/**
 * localizing_matrix_properties.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_properties.h"
#include "../localizing_matrix_index.h"

namespace Moment {

    class LocalizingMatrixProperties : public MatrixProperties {
    private:
        LocalizingMatrixIndex lmi;
    public:
        LocalizingMatrixProperties(MatrixProperties&& rhs, LocalizingMatrixIndex index,
                                   bool override_hermitian, std::string override_desc) noexcept
            : MatrixProperties{std::move(rhs)}, lmi{std::move(index)} {
            this->set_hermicity(override_hermitian);
            this->set_description(std::move(override_desc));
        }

        [[nodiscard]] constexpr const LocalizingMatrixIndex& LMI() const noexcept { return this->lmi; }

        [[nodiscard]] constexpr size_t Level() const noexcept { return this->lmi.Level; }

        [[nodiscard]] constexpr const auto& LocalizingWord() const noexcept { return this->lmi.Word; };
    };
}