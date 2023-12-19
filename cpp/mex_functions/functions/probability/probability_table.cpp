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

#include <tuple>

namespace Moment::mex::functions {

    namespace {
        [[nodiscard]] std::pair<ProbabilityTensorRange, const MaintainsTensors&>
        get_slice(matlab::engine::MATLABEngine& matlabEngine, const ProbabilityTableParams& input,
                  MatrixSystem& system, decltype(system.get_read_lock())& lock) {

            if (auto* lmsPtr = dynamic_cast<Locality::LocalityMatrixSystem*>(&system); lmsPtr != nullptr) {
                lmsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = lmsPtr->LocalityProbabilityTensor();

                PMConvertor pmReader{matlabEngine, lmsPtr->localityContext, true};
                auto free_mmts = pmReader.read_pm_index_list(input.free);
                auto fixed_mmts = pmReader.read_pmo_index_list(input.fixed);

                try {
                    return std::pair<ProbabilityTensorRange, const MaintainsTensors&>(pt.measurement_to_range(free_mmts, fixed_mmts), *lmsPtr);
                } catch (const Moment::errors::BadPTError& pte) {
                    throw_error(matlabEngine, errors::bad_param, pte.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            if (auto* imsPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system); imsPtr != nullptr) {
                imsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = imsPtr->InflationProbabilityTensor();

                OVConvertor ovReader{matlabEngine, imsPtr->InflationContext(), true};
                auto free_mmts = ovReader.read_ov_index_list(input.free);
                auto fixed_mmts = ovReader.read_ovo_index_list(input.fixed);

                try {
                    return {pt.measurement_to_range(free_mmts, fixed_mmts), *imsPtr};
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
            : SortedInputs{std::move(inputIn)}, matrix_system_key{matlabEngine} {

        // Get matrix system ID
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Get output mode if specified
        if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::OperatorSequences;
        } else if (this->flags.contains(u"full_sequences")) {
            this->output_mode = OutputMode::OperatorSequencesWithSymbolInfo;
        } else if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        }

        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_shape = ExportShape::WholeTensor;
            return;
        }

        // Otherwise, determine mode, and check dimensions
        if (this->inputs.size() == 2) {
            std::tie(this->free, this->fixed) = read_pairs_and_triplets(this->matlabEngine, this->inputs[1]);
        } else {
            assert(this->inputs.size() == 3);
            std::tie(this->free, this->fixed) = read_pairs_and_triplets(this->matlabEngine,
                                                                        this->inputs[1], this->inputs[2]);
        }
        this->export_shape = this->free.empty() ? ExportShape::OneOutcome : ExportShape::OneMeasurement;
    }

    ProbabilityTable::ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 3;

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"full_sequences");
        this->flag_names.emplace(u"sequences");
        this->mutex_params.add_mutex({u"symbols", u"sequences", u"full_sequences"});
    }


    void ProbabilityTable::operator()(IOArgumentRange output, ProbabilityTableParams &input) {

        // Get stored moment matrix
        auto msPtr = input.matrix_system_key(this->storageManager);
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
            case ProbabilityTableParams::OutputMode::OperatorSequencesWithSymbolInfo:
                output[0] = exporter.sequences_with_symbols(tensor);
                break;
            case ProbabilityTableParams::OutputMode::Symbols:
                output[0] = exporter.symbols(tensor);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
        }
    }

    void ProbabilityTable::export_one_measurement(IOArgumentRange output, ProbabilityTableParams &input,
                                                  MatrixSystem& raw_system) {
        auto lock = raw_system.get_read_lock();
        auto [slice, system] = get_slice(this->matlabEngine, input, raw_system, lock);

        ProbabilityTensorExporter exporter{this->matlabEngine, system};

        switch (input.output_mode) {
            case ProbabilityTableParams::OutputMode::OperatorSequences:
                output[0] = exporter.sequences(slice);
                break;

            case ProbabilityTableParams::OutputMode::OperatorSequencesWithSymbolInfo:
                output[0] = exporter.sequences_with_symbols(slice);
                break;

            case ProbabilityTableParams::OutputMode::Symbols:
                output[0] = exporter.symbols(slice);
                break;
            default:
                throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
        }
    }

    void ProbabilityTable::export_one_outcome(IOArgumentRange output, ProbabilityTableParams &input,
                                              MatrixSystem& raw_system) {
        auto lock = raw_system.get_read_lock();
        auto [slice, system] = get_slice(this->matlabEngine, input, raw_system, lock);

        ProbabilityTensorExporter exporter{this->matlabEngine, system};

        // Check there is one element referred to.
        auto iter = slice.begin();
        if (iter == slice.end()) {
            throw_error(matlabEngine, errors::internal_error, "Invalid measurement.");
        }

        matlab::data::CellArray one_by_one = exporter.factory.createCellArray({1, 1});
            (*one_by_one.begin()) = [&](const MaintainsTensors& the_system) -> matlab::data::CellArray {

            switch (input.output_mode) {
                case ProbabilityTableParams::OutputMode::OperatorSequences: {
                    auto fullPolyInfo = exporter.sequence(*iter, the_system.CollinsGisin());
                    return fullPolyInfo.move_to_cell(exporter.factory);
                }
                case ProbabilityTableParams::OutputMode::OperatorSequencesWithSymbolInfo: {
                    auto fullPolyInfo = exporter.sequence_with_symbols(*iter, the_system.CollinsGisin());
                    return fullPolyInfo.move_to_cell(exporter.factory);
                }
                case ProbabilityTableParams::OutputMode::Symbols:
                    return exporter.symbol(*iter);
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unknown output mode.");
            }
        }(system);
        output[0] = std::move(one_by_one);

    }
}