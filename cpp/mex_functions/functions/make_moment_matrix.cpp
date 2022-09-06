/**
 * make_moment_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_moment_matrix.h"

#include "storage_manager.h"

#include "operators/context.h"
#include "operators/matrix/moment_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "fragments/export_symbol_matrix.h"
#include "fragments/export_unique_sequences.h"

#include <memory>

namespace NPATK::mex::functions {

    MakeMomentMatrix::MakeMomentMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::MakeMomentMatrix, u"make_moment_matrix") {
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"reference");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"table");

        this->param_names.emplace(u"reference_id");
        this->param_names.emplace(u"level");

        // One of four ways to output:
        this->mutex_params.add_mutex(u"reference", u"sequences");
        this->mutex_params.add_mutex(u"reference", u"symbols");
        this->mutex_params.add_mutex(u"reference", u"table");
        this->mutex_params.add_mutex(u"sequences", u"symbols");
        this->mutex_params.add_mutex(u"sequences", u"table");
        this->mutex_params.add_mutex(u"symbols", u"table");

        // Either [ref, level] or named version thereof.
        this->min_inputs = 0;
        this->max_inputs = 2;
    }

    MakeMomentMatrixParams::MakeMomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {

        // Determine output mode:
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::Sequences;
        } else if (this->flags.contains(u"table")) {
            this->output_mode = OutputMode::TableOnly;
        } else {
            this->output_mode = OutputMode::Symbols;
        }

        // Either set named params OR give multiple params
        bool reference_specified = this->params.contains(u"reference_id");
        bool level_specified = this->params.contains(u"level");

        bool set_any_param = reference_specified || level_specified;

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            if (!level_specified) {
                throw errors::BadInput{errors::bad_param,
                                       "If a reference to matrix system is provided, 'level' must also be set."};
            }
            if (!reference_specified) {
                throw errors::BadInput{errors::bad_param,
                           "If a hierarchy level is given, 'reference_id' to the MatrixSystem must also be provided"};
            }


            // Get ref id
            auto& ref_param = this->find_or_throw(u"reference_id");
            this->storage_key = read_positive_integer(matlabEngine, "Parameter 'reference_id'", ref_param, 0);

            // Get depth
            auto& depth_param = this->find_or_throw(u"level");
            this->hierarchy_level = read_positive_integer(matlabEngine, "Parameter 'level'", depth_param, 0);

            // Done
            return;
        }

        // No named parameters... try to interpret inputs as Settings object + depth
        if (this->inputs.size() != 2) {
            throw errors::BadInput{errors::too_few_inputs,
                                   "Two inputs should be provided: [matrix system ID, level]."};
        }

        this->storage_key = read_positive_integer(matlabEngine, "MatrixSystem reference", inputs[0], 0);
        this->hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level", inputs[1], 0);
    }


    std::unique_ptr<SortedInputs>
    MakeMomentMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<MakeMomentMatrixParams>(this->matlabEngine, std::move(input));
        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }

        return output;
    }

    void MakeMomentMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<MakeMomentMatrixParams&>(*inputPtr);


        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        // Now, build or get moment matrix
        const auto& momentMatrix = matrixSystemPtr->CreateMomentMatrix(input.hierarchy_level);

        if (output.size() >= 1) {
            switch (input.output_mode) {
                case MakeMomentMatrixParams::OutputMode::Symbols:
                    output[0] = export_symbol_matrix(this->matlabEngine, momentMatrix.SymbolMatrix());
                    break;
                case MakeMomentMatrixParams::OutputMode::Sequences:
                    output[0] = export_sequence_matrix(this->matlabEngine, momentMatrix.context,
                                                       momentMatrix.SequenceMatrix());
                    break;
                case MakeMomentMatrixParams::OutputMode::TableOnly:
                    output[0] = export_unique_sequence_struct(this->matlabEngine, momentMatrix);
                    break;
                default:
                case MakeMomentMatrixParams::OutputMode::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output mode!");
            }
        }

        // Export symbol table, if second output specified
        if (output.size() >= 2) {
            if (input.output_mode == MakeMomentMatrixParams::OutputMode::TableOnly) {
                output[1] = output[0];
            } else {
                output[1] = export_unique_sequence_struct(this->matlabEngine, momentMatrix);
            }
        }

        // Also get dimensions, if third output specified
        if (output.size() >= 3) {
            matlab::data::ArrayFactory factory;
            output[2] = factory.createScalar<uint64_t>(momentMatrix.Dimension());
        }
    }
}