/**
 * matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "matrix.h"

#include "matrix_system.h"
#include "operator_matrix.h"

namespace Moment {

    Matrix::Matrix(const Context& context, SymbolTable& symbols, size_t dimension )
        : context{context}, symbol_table{symbols}, Symbols{symbols}, dimension{dimension}, Basis{*this} { }

    Matrix::~Matrix() noexcept = default;

    const OperatorMatrix &Matrix::operator_matrix() const {
        if (!has_operator_matrix()) {
            throw errors::missing_component{"No operator matrix defined for this matrix."};
        }
        return *this->op_mat;
    }

    Matrix::Matrix(Matrix &&rhs) noexcept: context{rhs.context}, symbol_table{rhs.symbol_table},
                                           dimension{rhs.dimension},
                                           mat_prop{std::move(rhs.mat_prop)},
                                           op_mat{std::move(rhs.op_mat)},
                                           Symbols{rhs.Symbols},
                                           Basis{*this, std::move(rhs.Basis)} { }

    void Matrix::set_description(std::string new_description) noexcept {
        assert(this->mat_prop);
        this->mat_prop->set_description(std::move(new_description));
    }
}