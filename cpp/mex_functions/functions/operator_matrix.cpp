/**
 * operator_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_matrix.h"

#include "storage_manager.h"

#include "matrix/operator_matrix.h"

#include "export/export_operator_matrix.h"
#include "export/export_matrix_basis_masks.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions  {

    namespace {

        void export_masks(matlab::engine::MATLABEngine &engine, IOArgumentRange& output,
                          const MatrixSystem& system, const SymbolicMatrix &matrix) {
            const auto num_outputs = output.size();
            auto read_lock = system.get_read_lock();

            // Export lists, if requested
            if (num_outputs > 2) {
                auto [re_list, im_list] = export_basis_lists(engine, system.Symbols(), matrix.SMP());
                // Guaranteed by above if:
                output[2] = std::move(re_list);

                if (num_outputs >= 4) {
                    output[3] = std::move(im_list);
                }
            }

            // Export masks
            auto [re_mask, im_mask] = export_basis_masks(engine, system.Symbols(), matrix.SMP());
            if (num_outputs >= 1) {
                output[0] = std::move(re_mask);
            }
            if (num_outputs >= 2) {
                output[1] = std::move(im_mask);
            }
        }
    }

    void OperatorMatrixParams::parse(matlab::engine::MATLABEngine &matlabEngine) {
        // Determine output mode:
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::Sequences;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        } else if (this->flags.contains(u"masks")) {
            this->output_mode = OutputMode::Masks;
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
            this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "Parameter 'reference_id'", ref_param, 0);

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
        this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference", inputs[0], 0);

        // Extra parsing
        this->extra_parse_inputs(matlabEngine);
    }

    bool OperatorMatrixParams::any_param_set() const {
        return this->params.contains(u"reference_id");
    }


    void RawOperatorMatrixParams::extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) {
        assert(inputs.empty()); // Should be guaranteed by parent.
        // Get depth
        auto& index_param = this->find_or_throw(u"index");
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Parameter 'index'", index_param, 0);
    }

    void RawOperatorMatrixParams::extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) {
        assert(this->inputs.size() == 2); // should be guaranteed by parent.
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Matrix index", inputs[1], 0);
    }

    bool RawOperatorMatrixParams::any_param_set() const {
        const bool index_specified = this->params.contains(u"index");
        return index_specified || OperatorMatrixParams::any_param_set();
    }


    OperatorMatrix::OperatorMatrix(matlab::engine::MATLABEngine &matlabEngine, Moment::mex::StorageManager &storage,
                                   MEXEntryPointID id, std::basic_string<char16_t> name)
            : MexFunction(matlabEngine, storage, id, std::move(name)) {
        this->min_outputs = 1;
        this->max_outputs = 4;

        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"dimension");
        this->flag_names.emplace(u"masks");

        this->param_names.emplace(u"reference_id");
        this->param_names.emplace(u"index");

        // One of four ways to output:
        this->mutex_params.add_mutex({u"sequences", u"symbols", u"dimension", u"masks"});

        // Either [ref, ref] or named version thereof.
        this->min_inputs = 0;
        this->max_inputs = 2;
    }


    std::unique_ptr<SortedInputs> OperatorMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<RawOperatorMatrixParams>(this->matlabEngine, std::move(input));
        output->parse(this->matlabEngine);

        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
        return output;
    }

    void OperatorMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<OperatorMatrixParams&>(*inputPtr);

        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.storage_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
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
                case OperatorMatrixParams::OutputMode::Sequences: {
                    output[0] = export_sequence_matrix(this->matlabEngine, matrixSystem, theMatrix);
                    }
                    break;
                case OperatorMatrixParams::OutputMode::IndexAndDimension: {
                    matlab::data::ArrayFactory factory;
                    output[0] = factory.createScalar<uint64_t>(matIndexPair.first);
                    if (output.size() >= 2) {
                        output[1] = factory.createScalar<uint64_t>(theMatrix.Dimension());
                    }
                }
                    break;
                case OperatorMatrixParams::OutputMode::Masks:
                    export_masks(this->matlabEngine, output, matrixSystem, theMatrix);
                    break;
                default:
                case OperatorMatrixParams::OutputMode::Unknown:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output mode!");
            }
        }
    }

    std::pair<size_t, const Moment::SymbolicMatrix&>
    OperatorMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) {
        try {
            const auto &input = dynamic_cast<const RawOperatorMatrixParams &>(omp);
            auto lock = system.get_read_lock(); // release on return or throw
            if (input.matrix_index >= system.size()) {
                throw_error(this->matlabEngine, errors::bad_param,
                            "Could not find matrix with index " + std::to_string(input.matrix_index)
                            + " in matrix system.");
            }
            return {input.matrix_index, system[input.matrix_index]};
        } catch (const std::bad_cast& bce) {
            throw_error(this->matlabEngine, errors::bad_cast,
                        "Misconfigured operator matrix input parameter object.");
        } catch (const Moment::errors::missing_component& mce) {
            throw_error(this->matlabEngine, errors::internal_error, mce.what());
        }
    }

    void OperatorMatrix::validate_output_count(const size_t outputs, const SortedInputs &rawInputs) const {
        const auto& input = dynamic_cast<const OperatorMatrixParams&>(rawInputs);
        switch(input.output_mode) {
            case OperatorMatrixParams::OutputMode::IndexAndDimension:
                if (outputs > 2) {
                    throw_error(this->matlabEngine, errors::too_many_outputs,
                                "At most two outputs should be provided for index (and dimension).");
                }
                break;
            case OperatorMatrixParams::OutputMode::Symbols:
            case OperatorMatrixParams::OutputMode::Sequences:
                if (outputs > 1) {
                    throw_error(this->matlabEngine, errors::too_many_outputs,
                                "Only one output should be provided for matrix export.");
                }
                break;
            case OperatorMatrixParams::OutputMode::Masks:
                if (outputs == 3) {
                    throw_error(this->matlabEngine, errors::too_many_outputs,
                                "Either one, two or four outputs should be provided for index (and mask) export");
                }
                break;
            case OperatorMatrixParams::OutputMode::Unknown:
            default:
                break;
        }
    }
}