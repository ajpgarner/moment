/**
 * matrix_properties.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "matrix_type.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Moment {

    class Matrix;
    class SymbolTable;

    /** Information about the particular matrix (relative to the collection). */
    class MatrixProperties {
    private:
        MatrixType basis_type = MatrixType::Unknown;
        size_t dimension;
        std::string description;
        std::set<symbol_name_t> included_symbols;
        std::set<symbol_name_t> real_entries;
        std::set<symbol_name_t> imaginary_entries;
        std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> elem_keys;

        /** True if matrix is complex Hermitian or real symmetric. */
        bool mat_is_herm = false;

    public:
        /** Construct symbolic properties from operator matrix. */
        MatrixProperties(const Matrix& matrix,
                         const SymbolTable& table,
                         std::set<symbol_name_t>&& subset,
                         const std::string& description,
                         bool is_hermitian);

        MatrixProperties(MatrixProperties&& rhs) = default;

        virtual ~MatrixProperties() noexcept = default;

        /** Set of all symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& IncludedSymbols() const noexcept {
            return this->included_symbols;
        }

        /** Use symbol table to sort included symbols into real and imaginary. */
        void rebuild_keys(const SymbolTable& table);

        /** Whether matrix is symmetric or Hermitian (i.e. does it contain imaginary symbols). */
        [[nodiscard]] constexpr MatrixType Type() const noexcept {
            return this->basis_type;
        }

        /** Size of this (square) matrix. */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /** String description of the matrix */
        [[nodiscard]] constexpr const std::string& Description() const noexcept {
            return this->description;
        }

        /** Set of real symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& RealSymbols() const noexcept {
            return this->real_entries;
        }

        /** Set of imaginary symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& ImaginarySymbols() const noexcept {
            return this->imaginary_entries;
        }

        /**
         * The basis keys for symbols in this matrix.
         */
        [[nodiscard]] const std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>>& BasisKey() const {
            return this->elem_keys;
        }

        /**
         * True if the matrix has complex elements (Hermitian or otherwise).
         */
        [[nodiscard]] bool IsComplex() const noexcept {
            return(this->basis_type == MatrixType::Complex) || (this->basis_type == MatrixType::Hermitian);
        }

        /**
         * True if the matrix has symmetry elements (real, or otherwise).
         */
        [[nodiscard]] bool IsHermitian() const noexcept {
            return this->mat_is_herm;
        }

    protected:
        void override_hermicity(bool is_hermitian) noexcept;

        void set_description(std::string new_description) noexcept;

    public:
        friend std::ostream& operator<<(std::ostream& os, const MatrixProperties& mp);

        friend class Matrix;
    };
}