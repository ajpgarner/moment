/**
 * plus.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "binary_operation.h"

namespace Moment::mex::functions  {

    using PlusParams = BinaryOperationParams;

    class Plus : public BinaryOperation<PlusParams, MTKEntryPointID::Plus> {
    public:
        explicit Plus(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : BinaryOperation<PlusParams, MTKEntryPointID::Plus>{matlabEngine, storage} { }

    protected:
        RawPolynomial one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) final;

        std::pair<ptrdiff_t, const SymbolicMatrix&>
        one_to_matrix(const MaintainsMutex::WriteLock& write_lock, const RawPolynomial& lhs,
                      const SymbolicMatrix& rhs) final;

        std::pair<ptrdiff_t, const SymbolicMatrix&>
        matrix_to_one(const MaintainsMutex::WriteLock& write_lock, const SymbolicMatrix& lhs,
                      const RawPolynomial& rhs) final {
            // NB: Addition commutes.
            return one_to_matrix(write_lock, rhs, lhs);
        }

        std::pair<ptrdiff_t, const SymbolicMatrix&>
        matrix_to_matrix(const MaintainsMutex::WriteLock& write_lock, const SymbolicMatrix& lhs,
                         const SymbolicMatrix& rhs) final;

    };
}
