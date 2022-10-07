/**
 * make_symmetric.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_symmetric.h"

#include "mex.hpp"

#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "symbolic/symbol.h"
#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include "fragments/export_substitution_list.h"
#include "fragments/identify_nonsymmetric_elements.h"
#include "fragments/substitute_elements_using_tree.h"
#include "fragments/read_symbol_or_fail.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"


namespace NPATK::mex::functions {

    MakeSymmetric::MakeSymmetric(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::MakeSymmetric, u"make_symmetric") {
        this->flag_names.emplace(u"dense");
        this->flag_names.emplace(u"sparse");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->min_outputs = 1;
        this->max_outputs = 2;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::unique_ptr<SortedInputs> MakeSymmetric::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            throw errors::BadInput{errors::bad_param, "Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            throw errors::BadInput{errors::bad_param, "Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Matrix type must be real numeric, or of strings."};
        }

        return std::make_unique<MakeSymmetricParams>(std::move(input));
    }


    MakeSymmetricParams::MakeSymmetricParams(SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {
        // Determine sparsity of output
        this->sparse_output = (inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE);
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }


    void MakeSymmetric::operator()(IOArgumentRange outputs, std::unique_ptr<SortedInputs> inputPtr) {
        auto& inputs = dynamic_cast<MakeSymmetricParams&>(*inputPtr);

        auto unique_constraints = identify_nonsymmetric_elements(matlabEngine, inputs.inputs[0]);

        if (verbose) {
            std::stringstream ss2;
            ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
                << unique_constraints.link_count() << " links.\n";
            if (debug) {
                ss2 << "Sorted, unique constraints:\n"
                    << unique_constraints;
            }
            NPATK::mex::print_to_console(matlabEngine, ss2.str());
        }

        unique_constraints.pack();
        auto symbol_tree = NPATK::SymbolTree{unique_constraints};

        if (debug) {
            std::stringstream ss3;
            ss3 << "\nTree, initial:\n" << symbol_tree;
            NPATK::mex::print_to_console(matlabEngine, ss3.str());
        }

        symbol_tree.simplify();

        if (verbose) {
            std::stringstream ss4;
            ss4 << "\nTree, simplified:\n" << symbol_tree << "\n";
            NPATK::mex::print_to_console(matlabEngine, ss4.str());
        }


        if (outputs.size() >= 1) {
            outputs[0] = NPATK::mex::make_symmetric_using_tree(matlabEngine, inputs.inputs[0],
                                                               symbol_tree, inputs.sparse_output);
        }

        if (outputs.size() >= 2) {
            outputs[1] = NPATK::mex::export_substitution_list(matlabEngine, symbol_tree);
        }
    }

}