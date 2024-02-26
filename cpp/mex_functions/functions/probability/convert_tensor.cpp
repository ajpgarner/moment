/**
 * convert_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "convert_tensor.h"

#include "utilities/read_choice.h"
#include "utilities/read_as_vector.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/tensor_conversion.h"
#include "scenarios/locality/party.h"

#include <algorithm>
#include <numeric>

namespace Moment::mex::functions {

    ConvertTensorParams::ConvertTensorParams(Moment::mex::SortedInputs&& inputs)
        : SortedInputs{std::move(inputs)} {

        // First, switch direction on first param
        auto choice = read_choice("Direction", {"cg2fc", "fc2cg"}, this->inputs[0]);
        if (choice == 0) {
            this->direction = Direction::GC_to_FC;
        } else {
            this->direction = Direction::FC_to_GC;
        }

        // Second, read second param as vector
        this->values = read_as_double_vector(this->matlabEngine, this->inputs[1]);

        // Next, from second parameter's shape make some deductions about party sizes.
        const auto input_dims = this->inputs[1].getDimensions();
        std::transform(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->mmts_per_party),
                       [this](size_t dim) {
            if (dim <= 1) {
                throw BadParameter{"Tensor must implicitly define at least one measurement per party."};
            }
            return dim - 1;
        });
    }

    ConvertTensor::ConvertTensor(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 2;
        this->max_inputs = 2;
    }

    void ConvertTensor::operator()(IOArgumentRange output, ConvertTensorParams &input) {

        // First, construct locality scenario context
        const size_t total_mmts = std::reduce(input.mmts_per_party.cbegin(),input.mmts_per_party.cend());
        std::vector<size_t> outcomes_per_mmt(total_mmts, static_cast<size_t>(2));
        Locality::LocalityContext context{Locality::Party::MakeList(input.mmts_per_party, outcomes_per_mmt)};

        // Initialize convertor
        Locality::TensorConvertor convertor{context};

        // Do conversion
        std::vector<double> result = (input.direction == ConvertTensorParams::Direction::GC_to_FC)
                                   ? convertor.collins_gisin_to_full_correlator(input.values)
                                   : convertor.full_correlator_to_collins_gisin(input.values);

        // Make output array
        matlab::data::ArrayDimensions output_dims;
        std::transform(input.mmts_per_party.cbegin(),input.mmts_per_party.cend(), std::back_inserter(output_dims),
                       [](const size_t dim) { return dim + 1; });
        matlab::data::ArrayFactory factory;
        output[0] = factory.createArray(std::move(output_dims), result.cbegin(), result.cend());
    }

}