/**
 * make_hermitian.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_hermitian.h"

#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include "fragments/export_substitution_list.h"
#include "fragments/export_symbol_tree_properties.h"
#include "fragments/identify_nonhermitian_elements.h"
#include "fragments/substitute_elements_using_tree.h"

#include "utilities/reporting.h"

namespace Moment::mex::functions  {

    MakeHermitian::MakeHermitian(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::MakeHermitian, u"make_hermitian") {
        this->min_outputs = 1;
        this->max_outputs = 3;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::unique_ptr<SortedInputs> MakeHermitian::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        // Should be guaranteed~
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            throw errors::BadInput{errors::bad_param, "Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            throw errors::BadInput{errors::bad_param, "Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Matrix type must be of strings."};
        }

        return std::move(inputPtr);
    }

    void MakeHermitian::operator()(IOArgumentRange outputs, std::unique_ptr<SortedInputs> inputPtr) {
        auto& inputs = *inputPtr;
        auto unique_constraints = identify_nonhermitian_elements(matlabEngine, inputs.inputs[0]);

        if (verbose) {
            std::stringstream ss2;
            ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
                << unique_constraints.link_count() << " links.\n";
            if (debug) {
                ss2 << "Sorted, unique constraints:\n"
                    << unique_constraints;
            }
            print_to_console(matlabEngine, ss2.str());
        }

        unique_constraints.pack();
        auto symbol_tree = SymbolTree{unique_constraints};

        if (debug) {
            std::stringstream ss3;
            ss3 << "\nTree, initial:\n" << symbol_tree;
            Moment::mex::print_to_console(matlabEngine, ss3.str());
        }

        symbol_tree.simplify();

        if (verbose) {
            std::stringstream ss4;
            ss4 << "\nTree, simplified:\n" << symbol_tree << "\n";
            Moment::mex::print_to_console(matlabEngine, ss4.str());
        }

        if (outputs.size() >= 1) {
            outputs[0] = Moment::mex::make_hermitian_using_tree(matlabEngine, inputs.inputs[0], symbol_tree);
        }

        if (outputs.size() >= 2) {
            outputs[1] = Moment::mex::export_substitution_list(matlabEngine, symbol_tree);
        }

        if (outputs.size() >= 3) {
            outputs[2] = Moment::mex::export_symbol_properties(matlabEngine, symbol_tree);
        }

    }
}