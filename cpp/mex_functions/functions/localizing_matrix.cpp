/**
 * localizing_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "localizing_matrix.h"

#include "storage_manager.h"

#include "operators/matrix/localizing_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "fragments/export_operator_matrix.h"
#include "fragments/export_symbol_table.h"

#include <memory>

namespace NPATK::mex::functions {
    LocalizingMatrixParams::LocalizingMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Determine output mode:
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::Sequences;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        } else {
            this->output_mode = OutputMode::DimensionOnly;
        }

        // Either set named params OR give multiple params
        bool reference_specified = this->params.contains(u"reference_id");
        bool level_specified = this->params.contains(u"level");
        bool word_specified = this->params.contains(u"word");

        bool set_any_param = reference_specified || level_specified || word_specified;

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            if (!level_specified) {
                throw errors::BadInput{errors::bad_param, "Parameter 'level' must be set."};
            }
            if (!reference_specified) {
                throw errors::BadInput{errors::bad_param,
                                       "Parameter 'reference_id' to the MatrixSystem must also be provided"};
            }
            if (!word_specified) {
                throw errors::BadInput{errors::bad_param,
                                       "Parameter 'word' must be set."};
            }


            // Get ref id
            auto& ref_param = this->find_or_throw(u"reference_id");
            this->storage_key = read_positive_integer(matlabEngine, "Parameter 'reference_id'", ref_param, 0);

            // Get depth
            auto& depth_param = this->find_or_throw(u"level");
            this->hierarchy_level = read_positive_integer(matlabEngine, "Parameter 'level'", depth_param, 0);

            // Get localizing word sequence
            auto& word_param = this->find_or_throw(u"word");
            this->localizing_word = read_integer_array(matlabEngine, "Parameter 'word'", word_param);

            // Done
            return;
        }

        // No named parameters... try to interpret inputs as Settings object + depth
        if (this->inputs.size() != 3) {
            throw errors::BadInput{errors::too_few_inputs,
                                   "Three inputs should be provided: [matrix system ID, level, word]."};
        }
        this->storage_key = read_positive_integer(matlabEngine, "MatrixSystem reference", inputs[0], 0);
        this->hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level", inputs[1], 0);
        this->localizing_word = read_integer_array(matlabEngine, "Localizing word", inputs[2]);

    }

    LocalizingMatrixIndex LocalizingMatrixParams::to_index(matlab::engine::MATLABEngine &matlabEngine,
                                                           const Context& context) const {
        // Check word is in range
        for (auto op : this->localizing_word) {
            if (op >= context.size()) {
                throw_error(matlabEngine, errors::bad_param, "Operator index in localizing word was out of range.");
            }
        }

        // Copy and construct LMI
        auto oper_copy = this->localizing_word;
        return LocalizingMatrixIndex{context, this->hierarchy_level, OperatorSequence{std::move(oper_copy), context}};
    }

    LocalizingMatrix::LocalizingMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::LocalizingMatrix, u"localizing_matrix") {
        this->min_outputs = 0;
        this->max_outputs = 1;

        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"dimension");

        this->param_names.emplace(u"reference_id");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        // One of three ways to output:
        this->mutex_params.add_mutex({u"sequences", u"symbols", u"dimension"});

        // Either [ref, level, word] or named version thereof.
        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    std::unique_ptr<SortedInputs>
    LocalizingMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<LocalizingMatrixParams>(this->matlabEngine, std::move(input));
        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }

        return output;
    }

    void LocalizingMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<LocalizingMatrixParams&>(*inputPtr);


        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        // First, make LM index
        auto index_lock = matrixSystemPtr->getReadLock();
        auto lmi = input.to_index(matlabEngine, matrixSystemPtr->Context());
        index_lock.unlock();

        // Now, build or get localizing matrix
        const auto& locMatrix = matrixSystemPtr->CreateLocalizingMatrix(lmi);

        // Output, if supplied.
        if (output.size() >= 1) {
            switch (input.output_mode) {
                case LocalizingMatrixParams::OutputMode::Symbols:
                    output[0] = export_symbol_matrix(this->matlabEngine, locMatrix.SymbolMatrix());
                    break;
                case LocalizingMatrixParams::OutputMode::Sequences:
                    output[0] = export_sequence_matrix(this->matlabEngine, locMatrix.context,
                                                       locMatrix.SequenceMatrix());
                    break;
                case LocalizingMatrixParams::OutputMode::DimensionOnly: {
                    matlab::data::ArrayFactory factory;
                    output[0] = factory.createScalar<uint64_t>(locMatrix.Dimension());
                }
                    break;
                default:
                case LocalizingMatrixParams::OutputMode::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output mode!");
            }
        }
    }
}