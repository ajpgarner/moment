/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "storage_manager.h"

#include "matrix/operator_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "export/export_sequence_matrix.h"
#include "export/export_symbol_matrix.h"
#include "export/export_matrix_basis_masks.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "scenarios/locality/locality_matrix_system.h"

namespace Moment::mex::functions  {

    namespace {

        void export_masks(matlab::engine::MATLABEngine &engine, IOArgumentRange& output,
                          const MatrixSystem& system, const Matrix &matrix) {
            const auto num_outputs = output.size();
            auto read_lock = system.get_read_lock();

            BasisKeyExporter bke{engine};

            // Export lists, if requested
            if (num_outputs > 2) {
                auto [re_list, im_list] = bke.basis_lists(matrix);
                // Guaranteed by above if:
                output[2] = std::move(re_list);

                if (num_outputs >= 4) {
                    output[3] = std::move(im_list);
                }
            }

            // Export masks
            auto [re_mask, im_mask] = bke.basis_masks(matrix);
            if (num_outputs >= 1) {
                output[0] = std::move(re_mask);
            }
            if (num_outputs >= 2) {
                output[1] = std::move(im_mask);
            }
        }
    }

    void OperatorMatrixParams::parse() {
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
            this->extra_parse_params();

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
        this->extra_parse_inputs();
    }

    bool OperatorMatrixParams::any_param_set() const {
        return this->params.contains(u"reference_id");
    }


    void RawOperatorMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent.
        // Get depth
        auto& index_param = this->find_or_throw(u"index");
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Parameter 'index'", index_param, 0);
    }

    void RawOperatorMatrixParams::extra_parse_inputs() {
        assert(this->inputs.size() == 2); // should be guaranteed by parent.
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Matrix index", inputs[1], 0);
    }

    bool RawOperatorMatrixParams::any_param_set() const {
        const bool index_specified = this->params.contains(u"index");
        return index_specified || OperatorMatrixParams::any_param_set();
    }

    void OperatorMatrixVirtualBase::check_mat_sys_id(OperatorMatrixParams &input) const {
        // Check key vs. storage manager
        if (!this->omvb_storageManager.MatrixSystems.check_signature(input.storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
    }

    void OperatorMatrixVirtualBase::do_validate_output_count(size_t outputs, const OperatorMatrixParams& input) const {
        switch(input.output_mode) {
            case OperatorMatrixParams::OutputMode::IndexAndDimension:
                if (outputs > 2) {
                    throw_error(this->omvb_matlabEngine, errors::too_many_outputs,
                                "At most two outputs should be provided for index (and dimension).");
                }
                break;
            case OperatorMatrixParams::OutputMode::Symbols:
            case OperatorMatrixParams::OutputMode::Sequences:
                if (outputs > 1) {
                    throw_error(this->omvb_matlabEngine, errors::too_many_outputs,
                                "Only one output should be provided for matrix export.");
                }
                break;
            case OperatorMatrixParams::OutputMode::Masks:
                if (outputs == 3) {
                    throw_error(this->omvb_matlabEngine, errors::too_many_outputs,
                                "Either one, two or four outputs should be provided for index (and mask) export");
                }
                break;
            case OperatorMatrixParams::OutputMode::Unknown:
            default:
                break;
        }
    }


    void OperatorMatrixVirtualBase::process(IOArgumentRange output, OperatorMatrixParams& input) {

        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->omvb_storageManager.MatrixSystems.get(input.storage_key);
        } catch(const Moment::errors::persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.storage_key << std::dec;
            throw_error(this->omvb_matlabEngine, errors::bad_param, errSS.str());
        }
        MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto matIndexPair = this->get_or_make_matrix(matrixSystem, input);

        const auto& theMatrix = matIndexPair.second;

        // Output, if supplied.
        if (output.size() >= 1) {
            auto lock = matrixSystem.get_read_lock();

            switch (input.output_mode) {
                case OperatorMatrixParams::OutputMode::Symbols: {
                    SymbolMatrixExporter exporter{this->omvb_matlabEngine};
                    if (theMatrix.is_monomial()) {
                        output[0] = exporter(dynamic_cast<const MonomialMatrix&>(theMatrix));
                    } else {
                        output[0] = exporter(dynamic_cast<const PolynomialMatrix&>(theMatrix));
                    }
                }
                    break;
                case OperatorMatrixParams::OutputMode::Sequences: {
                    SequenceMatrixExporter exporter{this->omvb_matlabEngine};
                    if (theMatrix.is_monomial()) {
                        const auto& monoMatrix = dynamic_cast<const MonomialMatrix&>(theMatrix);
                        const auto * locality_ms = dynamic_cast<const Locality::LocalityMatrixSystem*>(&matrixSystem);
                        if (locality_ms != nullptr) {
                            auto formatter = this->omvb_settings().get_locality_formatter();
                            assert(formatter);
                            output[0] = exporter(monoMatrix, *formatter);
                        } else {
                            output[0] = exporter(monoMatrix, matrixSystem);
                        }
                    } else {
                        const auto& polyMatrix = dynamic_cast<const PolynomialMatrix&>(theMatrix);
                        output[0] = exporter(polyMatrix, matrixSystem);
                    }
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
                    export_masks(this->omvb_matlabEngine, output, matrixSystem, theMatrix);
                    break;
                default:
                case OperatorMatrixParams::OutputMode::Unknown:
                    throw_error(omvb_matlabEngine, errors::internal_error, "Unknown output mode!");
            }
        }
    }

    std::pair<size_t, const Moment::Matrix&>
    RawOperatorMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) {
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
}