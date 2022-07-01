/**
 * implied_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "implied_symbols.h"

#include "storage_manager.h"

#include "matlab_classes/moment_matrix.h"
#include "operators/implicit_symbols.h"
#include "utilities/reporting.h"

namespace NPATK::mex::functions {

    ImpliedSymbols::ImpliedSymbols(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::ImpliedSymbols, u"implied_symbols") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void ImpliedSymbols::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        // Get input
        assert(inputPtr);
        const auto& input = dynamic_cast<const ImpliedSymbolsParams&>(*inputPtr);

        // Get stored moment matrix
        auto mmPtr = this->storageManager.MomentMatrices.get(input.moment_matrix_key);
        assert(mmPtr);
        const auto& momentMatrix = *mmPtr;

        // Create implied sequence object
        ImplicitSymbols implSym{momentMatrix};

        // TODO: EXPORT SYMBOLS

        throw_error(this->matlabEngine, errors::internal_error, "Not implemented.");
    }

    std::unique_ptr<SortedInputs> ImpliedSymbols::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto tx = std::make_unique<ImpliedSymbolsParams>(this->matlabEngine, std::move(*input));
        if (!this->storageManager.MomentMatrices.check_signature(tx->moment_matrix_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
        return tx;
    }

    ImpliedSymbolsParams::ImpliedSymbolsParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&input)
        : SortedInputs(std::move(input)) {

        auto [mmClassPtr, fail] = read_as_moment_matrix(matlabEngine, inputs[0]); // Implicit copy...
        if (!mmClassPtr) {
            throw errors::BadInput{errors::bad_param, fail.value()};
        }
        this->moment_matrix_key = mmClassPtr->Key();
    }
}