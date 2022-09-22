/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_table.h"
#include "storage_manager.h"

#include "fragments/export_symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

namespace NPATK::mex::functions {


    SymbolTableParams::SymbolTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs) {
        this->storage_key = read_positive_integer(matlabEngine, "MatrixSystem reference", inputs.inputs[0], 0);
        if (inputs.inputs.size() > 1) {
            this->output_mode = OutputMode::FromId;
            this->from_id = read_positive_integer(matlabEngine, "Symbol lower bound", inputs.inputs[1], 0);
        } else {
            this->output_mode = OutputMode::AllSymbols;
        }
    }


    SymbolTable::SymbolTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::SymbolTable, u"symbol_table") {
        this->min_outputs = this->max_outputs = 1;
        this->min_inputs = 1;
        this->max_inputs = 2;
    }

    std::unique_ptr<SortedInputs>
    SymbolTable::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<SymbolTableParams>(this->matlabEngine, std::move(input));
        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }

        return output;
    }

    void SymbolTable::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<SymbolTableParams&>(*inputPtr);

        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        // Get read lock on system
        std::shared_lock lock = matrixSystemPtr->getReadLock();

        // Extract symbol table and context from system
        const auto& context = matrixSystemPtr->Context();
        const auto& symbolTable = matrixSystemPtr->Symbols();

        // Export symbol table
        if (output.size() >= 1) {
            switch (input.output_mode) {
                case SymbolTableParams::OutputMode::AllSymbols:
                    output[0] = export_symbol_table_struct(this->matlabEngine, context, symbolTable);
                    break;
                case SymbolTableParams::OutputMode::FromId:
                    output[0] = export_symbol_table_struct(this->matlabEngine, context, symbolTable, input.from_id);
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output mode.");

            }
        }
    }
}