/**
 * convert_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mtk_function.h"


namespace Moment::mex::functions  {

    struct ConvertTensorParams : public SortedInputs {
    public:
        /** How do we want to export */
        enum struct Direction {
            /** Export specification of polynomials. */
            Unknown,
            /** Export specification of polynomials. */
            FC_to_GC,
            /** Export specification of polynomials, but also include symbol info. */
            GC_to_FC
        } direction = Direction::Unknown;

    public:
        explicit ConvertTensorParams(SortedInputs&& inputs);

        std::vector<size_t> mmts_per_party;
        std::vector<double> values;

    };

    class ConvertTensor : public ParameterizedMTKFunction<ConvertTensorParams, MTKEntryPointID::ConvertTensor> {
    public:
        explicit ConvertTensor(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ConvertTensorParams &input) override;


    };

}
