/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "storage_manager.h"

#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "export/export_operator_matrix.h"
#include "export/export_matrix_basis_masks.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "scenarios/locality/locality_matrix_system.h"

namespace Moment::mex::functions  {

    namespace {

        void export_masks(matlab::engine::MATLABEngine &engine, IOArgumentRange& output,
                          const MatrixSystem& system, const SymbolicMatrix &matrix) {
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

    const NameSet OperatorMatrixParams::output_mode_names{u"masks", u"monomial", u"name", u"properties",
                                                           u"polynomial", u"sequence_string",
                                                           u"symbol_string"};

    void OperatorMatrixParams::parse() {
        // Determine output mode:
        auto output_index = this->get_index_of_matched_flag(OperatorMatrixParams::output_mode_names);

        if (output_index.has_value()) {
            this->output_mode = static_cast<OutputMode>(output_index.value()+1); // 0 = unknown.
        } else {
            this->output_mode = OutputMode::Properties;
        }

        // Either set named params OR give multiple params
        bool reference_specified = this->params.contains(u"reference_id");
        bool set_any_param = this->any_param_set();

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw BadParameter{"Input arguments should be exclusively named, or exclusively unnamed."};
            }

            if (!reference_specified) {
                throw BadParameter{"Parameter 'reference_id' to the MatrixSystem must also be provided"};
            }

            // Get ref id
            this->find_and_parse_or_throw(u"reference_id", [this](auto& ref_param) {
                this->matrix_system_key.parse_input(ref_param);
            });

            // Extra parsing
            this->extra_parse_params();

            // Done
            return;
        }

        // No named parameters... try to interpret inputs as Settings object + depth
        size_t needed = this->inputs_required();

        if (this->inputs.size() != needed) {
            throw InputCountException{"operator_matrix", needed, needed, this->inputs.size(),
                                      "Input should be provided in form: " + this->input_format()};
        }
        this->matrix_system_key.parse_input(inputs[0]);

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
        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'index'", index_param, 0);
    }

    void RawOperatorMatrixParams::extra_parse_inputs() {
        assert(this->inputs.size() == 2); // should be guaranteed by parent.
        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Matrix index", inputs[1], 0);
    }

    bool RawOperatorMatrixParams::any_param_set() const {
        const bool index_specified = this->params.contains(u"index");
        return index_specified || OperatorMatrixParams::any_param_set();
    }

    void OperatorMatrixVirtualBase::do_validate_output_count(size_t outputs, const OperatorMatrixParams& input) const {
        switch(input.output_mode) {
            case OperatorMatrixParams::OutputMode::Properties:
                if (outputs > 4) {
                    throw OutputCountException{"operator_matrix", 1, 4, outputs,
                                               "At most four outputs should be provided for properties"};
                }
                break;
            case OperatorMatrixParams::OutputMode::Name:
            case OperatorMatrixParams::OutputMode::SequenceStrings:
            case OperatorMatrixParams::OutputMode::SymbolStrings:
                if (outputs > 1) {
                    throw OutputCountException{"operator_matrix", 1, 1, outputs,
                                "Only one output should be provided for matrix string export."};
                }
                break;
            case OperatorMatrixParams::OutputMode::Masks:
                if (outputs == 3) {
                        throw OutputCountException{"operator_matrix", 1, 1, 0, // throw 'too few' by default
                                "Either one, two or four outputs should be provided for index (and mask) export"};
                }
                break;
            case OperatorMatrixParams::OutputMode::Monomial:
                if ((outputs != 1) && (outputs != 7)) {
                    throw OutputCountException{"operator_matrix", 1, 1, 0, // throw 'too few' by default
                        "Either one or seven outputs should be provided for monomial export."};
                }
            case OperatorMatrixParams::OutputMode::Unknown:
            default:
                break;
        }
    }


    void OperatorMatrixVirtualBase::process(IOArgumentRange output, OperatorMatrixParams& input) {

        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.matrix_system_key(this->omvb_storageManager);
        MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto matIndexPair = this->get_or_make_matrix(matrixSystem, input);

        const auto& theMatrix = matIndexPair.second;

        // Output, if supplied.
        if (output.size() >= 1) {
            auto lock = matrixSystem.get_read_lock();

            // Make exporter object
            auto exporter = [&]() -> OperatorMatrixExporter {
                const auto * locality_ms = dynamic_cast<const Locality::LocalityMatrixSystem*>(&matrixSystem);
                if (locality_ms != nullptr) {
                    auto formatter = this->omvb_settings().get_locality_formatter();
                    assert(formatter);
                    return OperatorMatrixExporter{this->omvb_matlabEngine, *locality_ms, *formatter};
                }
                return OperatorMatrixExporter{this->omvb_matlabEngine, matrixSystem};
            }();

            // Do export
            switch (input.output_mode) {

                case OperatorMatrixParams::OutputMode::Masks:
                    export_masks(this->omvb_matlabEngine, output, matrixSystem, theMatrix);
                    break;

                case OperatorMatrixParams::OutputMode::Monomial: {
                    if (!theMatrix.is_monomial()) {
                        throw BadParameter{"Cannot output non-monomial matrix in monomial format."};
                    }
                    auto monomial = exporter.monomials(dynamic_cast<const MonomialMatrix &>(theMatrix));
                    if (output.size() == 1) {
                        output[0] = monomial.move_to_cell(exporter.factory);
                    } else {
                        monomial.move_to_output(output);
                    }
                }
                    break;

                case OperatorMatrixParams::OutputMode::Name:
                    output[0] = exporter.name(theMatrix);
                    break;

                case OperatorMatrixParams::OutputMode::Properties:
                    exporter.properties(output, matIndexPair.first, theMatrix);
                    break;

                case OperatorMatrixParams::OutputMode::Polynomial:
                    output[0] = exporter.polynomials(theMatrix);
                    break;

                case OperatorMatrixParams::OutputMode::SequenceStrings:
                    output[0] = exporter.sequence_strings(theMatrix);
                    break;

                case OperatorMatrixParams::OutputMode::SymbolStrings:
                    output[0] = exporter.symbol_strings(theMatrix);
                    break;

                default:
                case OperatorMatrixParams::OutputMode::Unknown:
                    throw InternalError{"Unknown output mode!"};
            }
        }
    }

    std::pair<size_t, const Moment::SymbolicMatrix&>
    RawOperatorMatrix::get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) {
        try {
            const auto &input = dynamic_cast<const RawOperatorMatrixParams &>(omp);
            auto lock = system.get_read_lock(); // release on return or throw
            if (input.matrix_index >= system.size()) {
                throw BadParameter{"Could not find matrix with index " + std::to_string(input.matrix_index)
                                   + " in matrix system."};
            }
            return {input.matrix_index, system[input.matrix_index]};
        } catch (const std::bad_cast& bce) {
            throw BadCastException{"Misconfigured operator matrix input parameter object."};
        } catch (const Moment::errors::missing_component& mce) {
            throw InternalError{mce.what()};
        }
    }
}