/**
 * make_hermitian.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_hermitian.h"

namespace NPATK::mex::functions  {

    MakeHermitian::MakeHermitian(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeSymmetric, u"make_hermitian") {
        this->flag_names.emplace(u"dense");
        this->flag_names.emplace(u"sparse");

        this->min_outputs = 1;
        this->max_outputs = 2;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::pair<bool, std::basic_string<char16_t>> MakeHermitian::validate_inputs(const SortedInputs &input) const {
        // Should be guaranteed~
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            return {false, u"Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            return {false, u"Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                return {false, u"Matrix type must be of strings."};
        }

        if (input.flags.contains(u"dense") && input.flags.contains(u"sparse")) {
            return {false, u"Only one of \"dense\" or \"sparse\" should be set."};
        }

        return {true, u""};
    }

    void MakeHermitian::operator()(FlagArgumentRange outputs, SortedInputs&& inputs) {
        bool debug = (inputs.flags.contains(u"debug"));
        bool verbose = debug || (inputs.flags.contains(u"verbose"));
    }
}