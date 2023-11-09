/**
 * symbolic_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbolic_matrix.h"

#include "matrix_system/matrix_system.h"
#include "operator_matrix/operator_matrix.h"

#include <iostream>

namespace Moment {

    SymbolicMatrix::SymbolicMatrix(const Context& context, SymbolTable& symbols, size_t dimension )
        : context{context}, symbols{symbols}, symbol_table{symbols}, dimension{dimension}, Basis{*this} {

        if (debug_mode) {
            this->description = "Abstract Matrix";
        }
    }

    SymbolicMatrix::~SymbolicMatrix() noexcept = default;

    const OperatorMatrix &SymbolicMatrix::operator_matrix() const {
        if (!has_operator_matrix()) {
            throw errors::missing_component{"No operator matrix defined for this matrix."};
        }
        return *this->op_mat;
    }

    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::pre_multiply(const Monomial &lhs,  SymbolTable& symbol_registry,
                                 const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{"Pre-multiplication not defined for generic SymbolicMatrix."};
    }

    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::post_multiply(const Monomial &rhs,  SymbolTable& symbol_registry,
                                  const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{"Post-multiplication not defined for generic SymbolicMatrix."};
    }

    std::ostream& operator<<(std::ostream& os, const SymbolicMatrix& mp) {
        os << mp.dimension << "x" << mp.dimension << " ";
        if (mp.complex_basis) {
            if (mp.hermitian) {
                os << "Hermitian matrix";
            } else {
                os << "Complex matrix";
            }
        } else {
            if (mp.hermitian) {
                os << "Symmetric matrix";
            } else {
                os << "Real matrix";
            }
        }
        const auto num_us = mp.included_symbols.size();
        os << " with "
           << num_us << " unique " << (num_us != 1 ? "symbols" : "symbol");
        const auto num_re = mp.real_basis_elements.size();
        if (num_re > 0) {
            os << ", " << num_re << " real";
        }
        const auto num_im = mp.imaginary_basis_elements.size();
        if (num_im > 0) {
            os << ", " << num_im << " imaginary";
        }
        os << ".";
        return os;
    }


}