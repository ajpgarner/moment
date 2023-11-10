/**
 * multiply.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */


#include "../../mtk_function.h"
#include "integer_types.h"

#include <span>
#include <string>

namespace Moment {
    class Context;
}

namespace Moment::mex::functions  {

    struct MultiplyParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;


    public:
        explicit MultiplyParams(SortedInputs&& inputs);

    };

    class Multiply : public ParameterizedMTKFunction<MultiplyParams, MTKEntryPointID::Multiply> {
    public:
        explicit Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MultiplyParams &input) override;

        void extra_input_checks(MultiplyParams &input) const override;

    private:
        /** Raise error if operator string is bad. */
        void validate_op_seq(const Context& context,
                             std::span<const oper_name_t> operator_string,
                             size_t index = 0) const;
    };
}
