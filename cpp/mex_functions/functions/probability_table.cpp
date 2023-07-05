/**
 * probability_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "probability_table.h"

#include "storage_manager.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_probability_tensor.h"

#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_probability_tensor.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "export/export_probability_tensor.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    namespace {
        [[nodiscard]] ProbabilityTensorRange get_slice(matlab::engine::MATLABEngine& matlabEngine,
                                                                          const ProbabilityTableParams& input,
                                                                          MatrixSystem& system,
                                                                          decltype(system.get_read_lock())& lock) {

            if (auto* lmsPtr = dynamic_cast<Locality::LocalityMatrixSystem*>(&system); lmsPtr != nullptr) {
                lmsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = lmsPtr->LocalityProbabilityTensor();

                PMConvertor pmReader{matlabEngine, lmsPtr->localityContext};
                auto free_mmts = pmReader.read_pm_index_list(input.free);
                auto fixed_mmts = pmReader.read_pmo_index_list(input.fixed);

                try {
                    return pt.measurement_to_range(free_mmts, fixed_mmts);
                } catch (const Moment::errors::BadPTError& pte) {
                    throw_error(matlabEngine, errors::bad_param, pte.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            if (auto* imsPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system); imsPtr != nullptr) {
                imsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = imsPtr->InflationProbabilityTensor();

                OVConvertor ovReader{matlabEngine, imsPtr->InflationContext()};
                auto free_mmts = ovReader.read_ov_index_list(input.free);
                auto fixed_mmts = ovReader.read_ovo_index_list(input.fixed);

                try {
                    return pt.measurement_to_range(free_mmts, fixed_mmts);
                } catch (const Moment::errors::BadPTError& pte) {
                    throw_error(matlabEngine, errors::bad_param, pte.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            throw_error(matlabEngine, errors::bad_param, "Matrix system must be a locality or inflation system.");
        }
    }

    ProbabilityTableParams::ProbabilityTableParams(SortedInputs &&inputIn)
            : SortedInputs(std::move(inputIn)) {
        // Get matrix system ID
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // Get output mode if specified
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::OperatorSequences;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        }

        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_shape = ExportShape::WholeTensor;
            return;
        }

        // Otherwise, determine mode, and check dimensions
        bool hasFreeMmts = false;
        bool hasFixedMmts = false;
        if (this->inputs.size() == 3) {
            if (this->inputs[1].getNumberOfElements() != 0) {
                auto inputOneDims = this->inputs[1].getDimensions();
                if ((inputOneDims.size() != 2) || (inputOneDims[1] != 2)) {
                    throw_error(this->matlabEngine, errors::bad_param, "Measurement list should be a Nx2 array.");
                }
                hasFreeMmts = true;
            } else {
                hasFreeMmts = false;
            }
            if (this->inputs[2].getNumberOfElements() != 0) {
                auto inputTwoDims = this->inputs[2].getDimensions();
                if ((inputTwoDims.size() != 2) || (inputTwoDims[1] != 3)) {
                    throw_error(this->matlabEngine, errors::bad_param, "Outcome list should be a Nx3 array.");
                }
                hasFixedMmts = true;
            } else {
                hasFixedMmts = false;
            }
        } else {
            auto onlyInputDims = this->inputs[1].getDimensions();
            if (onlyInputDims.size() != 2) {
                throw_error(this->matlabEngine, errors::bad_param, "Measurement/outcome list should be a 2D array.");
            }
            if (onlyInputDims[1] == 2) {
                hasFreeMmts = true;
                hasFixedMmts = false;
            } else if (onlyInputDims[1] == 3) {
                hasFreeMmts = false;
                hasFixedMmts = true;
            } else {
                throw_error(this->matlabEngine, errors::bad_param,
                            "Measurement list should be a Nx2 array, outcome list a Nx3 array.");
            }
        }

        // Do read
        if (hasFreeMmts) {
            this->free = RawIndexPair::read_list(matlabEngine, inputs[1]);
            this->export_shape = ExportShape::OneMeasurement;
        } else {
            this->export_shape = ExportShape::OneOutcome;
        }

        if (hasFixedMmts) {
            this->fixed = RawIndexTriplet::read_list(matlabEngine, (this->inputs.size() == 2) ? inputs[1] : inputs[2]);
        }
    }

    ProbabilityTable::ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 3;

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->mutex_params.add_mutex(u"symbols", u"sequences");
    }


    void ProbabilityTable::extra_input_checks(ProbabilityTableParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }

    void ProbabilityTable::operator()(IOArgumentRange output, ProbabilityTableParams &input) {
        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent
        auto& system = *msPtr;

        switch (input.export_shape) {
            case ProbabilityTableParams::ExportShape::WholeTensor:
                export_whole_tensor(output, input, system);
                return;
            case ProbabilityTableParams::ExportShape::OneMeasurement:
                export_one_measurement(output, input, system);
                return;
            case ProbabilityTableParams::ExportShape::OneOutcome:
                export_one_outcome(output, input, system);
                return;
        }
    }

    void ProbabilityTable::export_whole_tensor(IOArgumentRange output, ProbabilityTableParams &input,
                                               MatrixSystem& system) {
        auto* ptSystemPtr = dynamic_cast<MaintainsTensors*>(&system);
        if (nullptr == ptSystemPtr) {
            throw_error(this->matlabEngine, errors::bad_param, "MatrixSystem does not maintain a probability tensor.");
        }
        auto lock = ptSystemPtr->get_read_lock();
        ptSystemPtr->RefreshProbabilityTensor(lock);
        const auto& tensor = ptSystemPtr->ProbabilityTensor();

        ProbabilityTensorExporter exporter{this->matlabEngine, system};
        switch (input.output_mode) {
            case ProbabilityTableParams::OutputMode::OperatorSequences:
                output[0] = exporter.sequences(tensor);
                break;
            case ProbabilityTableParams::OutputMode::Symbols:
                output[0] = exporter.symbols(tensor);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
        }
    }

    void ProbabilityTable::export_one_measurement(IOArgumentRange output, ProbabilityTableParams &input,
                                                  MatrixSystem& system) {
        auto lock = system.get_read_lock();
        auto slice = get_slice(this->matlabEngine, input, system, lock);

        ProbabilityTensorExporter exporter{this->matlabEngine, system};

        switch (input.output_mode) {
            case ProbabilityTableParams::OutputMode::OperatorSequences:
                output[0] = exporter.sequences(slice);
                break;
            case ProbabilityTableParams::OutputMode::Symbols:
                output[0] = exporter.symbols(slice);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
        }
    }

    void ProbabilityTable::export_one_outcome(IOArgumentRange output, ProbabilityTableParams &input,
                                              MatrixSystem& system) {
        auto lock = system.get_read_lock();
        auto slice = get_slice(this->matlabEngine, input, system, lock);

        ProbabilityTensorExporter exporter{this->matlabEngine, system};

        // Check there is one element referred to.
        auto iter = slice.begin();
        if (iter == slice.end()) {
            throw_error(matlabEngine, errors::internal_error, "Invalid measurement.");
        }

        switch (input.output_mode) {
            case ProbabilityTableParams::OutputMode::OperatorSequences:
                output[0] = exporter.sequence(*iter);
                break;
            case ProbabilityTableParams::OutputMode::Symbols:
                output[0] = exporter.symbol(*iter);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
        }
    }
}