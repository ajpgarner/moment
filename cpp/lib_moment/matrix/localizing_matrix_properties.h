/**
 * localizing_matrix_properties.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_properties.h"
#include "localizing_matrix_index.h"

namespace Moment {

    class LocalizingMatrixProperties : public MatrixProperties {
    private:
        LocalizingMatrixIndex lmi;
    public:
        LocalizingMatrixProperties(const Matrix& matrix, const SymbolTable& table, std::set<symbol_name_t>&& subset,
                                                 const std::string& description, bool is_hermitian,
                                                 LocalizingMatrixIndex index)
            : MatrixProperties{matrix, table, std::move(subset), description, is_hermitian}, lmi{std::move(index)} { }

        LocalizingMatrixProperties(MatrixProperties&& rhs, LocalizingMatrixIndex index,
                                   bool override_hermitian, std::string override_desc) noexcept
            : MatrixProperties{std::move(rhs)}, lmi{std::move(index)} {
            this->override_hermicity(override_hermitian);
            this->set_description(std::move(override_desc));
        }

        [[nodiscard]] constexpr const LocalizingMatrixIndex& LMI() const noexcept { return this->lmi; }

        [[nodiscard]] constexpr size_t Level() const noexcept { return this->lmi.Level; }

        [[nodiscard]] constexpr const auto& LocalizingWord() const noexcept { return this->lmi.Word; };
    };
}