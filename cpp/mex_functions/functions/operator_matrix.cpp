/**
 * operator_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_matrix.h"

#include "storage_manager.h"

#include "operators/matrix/operator_matrix.h"

#include "utilities/reporting.h"
#include "fragments/export_operator_matrix.h"

namespace NPATK::mex::functions  {


    void OperatorMatrixParams::parse(matlab::engine::MATLABEngine &matlabEngine) {
        // Determine output mode:
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::Sequences;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        } else {
            this->output_mode = OutputMode::IndexAndDimension;
        }

        // Either set named params OR give multiple params
        bool reference_specified = this->params.contains(u"reference_id");
        bool set_any_param = this->any_param_set();

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            if (!reference_specified) {
                throw errors::BadInput{errors::bad_param,
                                       "Parameter 'reference_id' to the MatrixSystem must also be provided"};
            }

            // Get ref id
            auto& ref_param = this->find_or_throw(u"reference_id");
            this->storage_key = read_positive_integer(matlabEngine, "Parameter 'reference_id'", ref_param, 0);

            // Extra parsing
            this->extra_parse_params(matlabEngine);

            // Done
            return;
        }

        // No named parameters... try to interpret inputs as Settings object + depth
        size_t needed = this->inputs_required();

        if (this->inputs.size() < needed) {
            throw errors::BadInput{errors::too_few_inputs,
                                   "Input should be provided in form: " + this->input_format()};
        } else if (this->inputs.size() > needed) {
            throw errors::BadInput{errors::too_many_inputs,
                                   "Input should be provided in form: " + this->input_format()};
        }
        this->storage_key = read_positive_integer(matlabEngine, "MatrixSystem reference", inputs[0], 0);

        // Extra parsing
        this->extra_parse_inputs(matlabEngine);
    }

    bool OperatorMatrixParams::any_param_set() const {
        return this->params.contains(u"reference_id");
    }



    OperatorMatrix::OperatorMatrix(matlab::engine::MATLABEngine &matlabEngine, NPATK::mex::StorageManager &storage,
                                   MEXEntryPointID id, std::basic_string<char16_t> name)
            : MexFunction(matlabEngine, storage, id, std::move(name)) {
        this->min_outputs = 0;
        this->max_outputs = 2;

        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"dimension");

        this->param_names.emplace(u"reference_id");

        // One of three ways to output:
        this->mutex_params.add_mutex({u"sequences", u"symbols", u"dimension"});

        // Either [ref, level, word] or named version thereof.
        this->min_inputs = 0;
        this->max_inputs = 3;
    }


    void OperatorMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<OperatorMatrixParams&>(*inputPtr);

        if ((output.size() >= 2) && (input.output_mode !=  OperatorMatrixParams::OutputMode::IndexAndDimension))  {
            throw_error(this->matlabEngine, errors::too_many_outputs,
                        "Too many outputs supplied.");
        }

        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }
        MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto matIndexPair = this->get_or_make_matrix(matrixSystem, input);
        const auto& theMatrix = matIndexPair.second;

        // Output, if supplied.
        if (output.size() >= 1) {
            switch (input.output_mode) {
                case OperatorMatrixParams::OutputMode::Symbols:
                    output[0] = export_symbol_matrix(this->matlabEngine, theMatrix.SymbolMatrix());
                    break;
                case OperatorMatrixParams::OutputMode::Sequences:
                    output[0] = export_sequence_matrix(this->matlabEngine, theMatrix.context,
                                                       theMatrix.SequenceMatrix());
                    break;
                case OperatorMatrixParams::OutputMode::IndexAndDimension: {
                    matlab::data::ArrayFactory factory;
                    output[0] = factory.createScalar<uint64_t>(matIndexPair.first);
                    if (output.size() >= 2) {
                        output[1] = factory.createScalar<uint64_t>(theMatrix.Dimension());
                    }
                }
                    break;
                default:
                case OperatorMatrixParams::OutputMode::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output mode!");
            }
        }
    }

}