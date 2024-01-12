/**
 * multiply.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "binary_operation.h"

namespace Moment {
    class Context;
    class MatrixSystem;
}

namespace Moment::mex::functions  {

    struct MultiplyParams final : public BinaryOperationParams {
    public:
        explicit MultiplyParams(SortedInputs&& inputs);
    };

    class Multiply : public BinaryOperation<MultiplyParams, MTKEntryPointID::Multiply> {
    public:
        explicit Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : BinaryOperation<MultiplyParams, MTKEntryPointID::Multiply>{matlabEngine, storage} {

        }

    protected:
        RawPolynomial one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) final;

        std::pair<ptrdiff_t, const SymbolicMatrix&>
        one_to_matrix(const MaintainsMutex::WriteLock& write_lock, const RawPolynomial& lhs,
                      const SymbolicMatrix& rhs) override;

        std::pair<ptrdiff_t, const SymbolicMatrix&>
        matrix_to_one(const MaintainsMutex::WriteLock& write_lock, const SymbolicMatrix& lhs,
                      const RawPolynomial& rhs) override;

    };
}
