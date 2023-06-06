/**
 * matrix_properties.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

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
        /** Dimension of the matrix */
        size_t dimension;

        /** True if matrix has any complex coefficients in front of its elements (real or otherwise) */
        bool mat_has_complex_coefficients = false;

        /** True if matrix has any complex elements */
        bool mat_is_complex = false;

        /** True if matrix is complex Hermitian or real symmetric. */
        bool mat_is_herm = false;

        /** Human-readable name for matrix */
        std::string description;

        /** The symbols involved in the matrix */
        std::set<symbol_name_t> included_symbols;

        /** ??? */
        std::set<symbol_name_t> real_entries;

        /** ??? */
        std::set<symbol_name_t> imaginary_entries;

        /** ??? */
        std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> elem_keys;


    public:
        /** Construct symbolic properties from operator matrix. */
        MatrixProperties(const Matrix& matrix,
                         const SymbolTable& table,
                         std::set<symbol_name_t>&& subset,
                         const std::string& description,
                         bool has_complex_coefficients,
                         bool is_hermitian);

        MatrixProperties(MatrixProperties&& rhs) = default;

        virtual ~MatrixProperties() noexcept = default;

        /** Size of this (square) matrix. */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /**
         * True if the matrix has complex elements (Hermitian or otherwise).
         */
        [[nodiscard]] constexpr bool IsComplex() const noexcept {
            return this->mat_is_complex;
        }

        /**
         * True if the matrix has symmetry elements (real, or otherwise).
         */
        [[nodiscard]] constexpr bool IsHermitian() const noexcept {
            return this->mat_is_herm;
        }

        /** String description of the matrix */
        [[nodiscard]] constexpr const std::string& Description() const noexcept {
            return this->description;
        }

        /** Set of all symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& IncludedSymbols() const noexcept {
            return this->included_symbols;
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

        /** Use symbol table to sort included symbols into real and imaginary parts. */
        void rebuild_keys(const SymbolTable& table);

    protected:
        /** Set whether the matrix is Hermitian/symmetric */
        void set_hermicity(bool is_hermitian) noexcept {
            this->mat_is_herm = is_hermitian;
        }

        /** Set the name of the matrix */
        void set_description(std::string new_description) noexcept {
            this->description = std::move(new_description);
        }

    public:
        friend std::ostream& operator<<(std::ostream& os, const MatrixProperties& mp);

        friend class Matrix;
    };
}