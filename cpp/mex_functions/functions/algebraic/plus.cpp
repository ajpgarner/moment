/**
 * plus.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "plus.h"
#include "binary_operation_impl.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

namespace Moment::mex::functions {

    // Explicitly instantiate binary operation functions:
    template class BinaryOperation<PlusParams, MTKEntryPointID::Plus>;

    RawPolynomial Plus::one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) {
        return RawPolynomial::add(lhs, rhs, this->pf_ptr->zero_tolerance);
    }

    std::pair<ptrdiff_t, const SymbolicMatrix&>
    Plus::one_to_matrix(const MaintainsMutex::WriteLock& write_lock, const RawPolynomial& lhs,
                        const SymbolicMatrix& rhs) {

        assert(this->ms_ptr->is_locked_write_lock(write_lock));

        const auto& factory = *this->pf_ptr;
        auto& symbols = this->ms_ptr->Symbols();

        // Promote raw polynomial to symbolic polynomial (registering new symbols as necessary)
        Polynomial symbolic_lhs = lhs.to_polynomial_register_symbols(factory, symbols);

        // Do addition
        auto added_matrix_ptr = rhs.add(symbolic_lhs, factory, Multithreading::MultiThreadPolicy::Optional);

        // Add to matrix system
        auto& system = *this->ms_ptr;
        const ptrdiff_t matrix_index = system.push_back(write_lock, std::move(added_matrix_ptr));

        // Report
        const auto& the_matrix = system.get(matrix_index);
        return std::pair<ptrdiff_t, const SymbolicMatrix&>{matrix_index, the_matrix};

    }

    std::pair<ptrdiff_t, const SymbolicMatrix&>
    Plus::matrix_to_matrix(const MaintainsMutex::WriteLock& write_lock,
                           const SymbolicMatrix& lhs,
                           const SymbolicMatrix& rhs) {
        // Complain if matrices do not match
        if (lhs.Dimension() != rhs.Dimension()) {
            throw_error(this->matlabEngine, errors::bad_param,
                        "When summands are matrices, their dimensions must match.");
        }

        // Do addition
        auto added_matrix_ptr = lhs.add(rhs, *this->pf_ptr, Multithreading::MultiThreadPolicy::Optional);

        // Add to matrix system
        auto& system = *this->ms_ptr;
        const ptrdiff_t matrix_index = system.push_back(write_lock, std::move(added_matrix_ptr));

        // Report
        const auto& the_matrix = system.get(matrix_index);
        return std::pair<ptrdiff_t, const SymbolicMatrix&>{matrix_index, the_matrix};
    }
}