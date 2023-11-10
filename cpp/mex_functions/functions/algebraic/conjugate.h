/**
 * conjugate.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "mtk_function.h"

#include "integer_types.h"

#include <span>
#include <vector>

namespace Moment {
    class Context;
}

namespace Moment::mex::functions {

    struct ConjugateParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The operator string(s) to conjugate. */
        std::vector<std::vector<oper_name_t>> operator_string;

        /** Input shape (1,1 for scalar) */
        std::vector<size_t> input_shape;

    public:
        explicit ConjugateParams(SortedInputs&& structuredInputs);

        [[nodiscard]] bool scalar_input() const noexcept;

    };

    class Conjugate : public ParameterizedMTKFunction<ConjugateParams, MTKEntryPointID::Conjugate> {
    public:
        explicit Conjugate(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ConjugateParams &input) override;

        void extra_input_checks(ConjugateParams &input) const override;

    private:
        /** Raise error if operator string is bad. */
        void validate_op_seq(const Context& context, std::span<const oper_name_t> operator_string, size_t index = 0) const;
    };

}