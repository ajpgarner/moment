/**
 * full_correlator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "full_correlator.h"

#include "storage_manager.h"

#include "export/export_full_correlator.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "probability/full_correlator.h"
#include "probability/maintains_tensors.h"

#include "scenarios/locality/locality_full_correlator.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "scenarios/inflation/inflation_full_correlator.h"
#include "scenarios/inflation/inflation_matrix_system.h"


namespace Moment::mex::functions  {
    namespace {
        [[nodiscard]] PolynomialElement get_element(matlab::engine::MATLABEngine& matlabEngine,
                                                    const FullCorrelatorParams& input,
                                                    MaintainsTensors& system,
                                                    decltype(system.get_read_lock())& lock) {

            if (auto* lmsPtr = dynamic_cast<Locality::LocalityMatrixSystem*>(&system); lmsPtr != nullptr) {
                lmsPtr->RefreshProbabilityTensor(lock);
                const auto& fc = lmsPtr->LocalityFullCorrelator();

                PMConvertor pmReader{matlabEngine, lmsPtr->localityContext, true};
                auto mmt_indices = pmReader.read_pm_index_list(input.measurementIndices);

                try {
                    return fc.mmt_to_element(mmt_indices);
                } catch (const Moment::errors::bad_tensor& pte) {
                    throw BadParameter{pte.what()};
                } catch (const std::exception& e) {
                    throw InternalError{e.what()};
                }
            }

            if (auto* imsPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system); imsPtr != nullptr) {
                imsPtr->RefreshProbabilityTensor(lock);
                const auto& fc = imsPtr->InflationFullCorrelator();

                OVConvertor ovReader{matlabEngine, imsPtr->InflationContext(), true};
                auto mmt_indices = ovReader.read_ov_index_list(input.measurementIndices);

                throw InternalError{"Not yet supported for inflation scenario."};
            }

            throw BadParameter{"Matrix system must be a locality or inflation system."};
        }
    }

    FullCorrelatorParams::FullCorrelatorParams(Moment::mex::SortedInputs&& inputIn)
            : SortedInputs{std::move(inputIn)}, matrix_system_key{matlabEngine} {

        // Get matrix system class
        this->matrix_system_key.parse_input(this->inputs[0]);

        // See if output type is set
        if (this->flags.contains(u"symbols")) {
            this->output_mode = OutputMode::Symbols;
        } else if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::OperatorSequences;
        } else if (this->flags.contains(u"full_sequences")) {
            this->output_mode = OutputMode::OperatorSequencesWithSymbolInfo;
        } else if (this->flags.contains(u"strings")) {
            this->output_mode = OutputMode::Strings;
        }

        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_shape = ExportShape::WholeTensor;
            return;
        }

        // Otherwise, determine mode, and check dimensions
        if (this->inputs.size() == 2) {
            this->measurementIndices = RawIndexPair::read_list(this->matlabEngine, this->inputs[1]);
            this->export_shape = ExportShape::OneCorrelator;
        }

    }

    FullCorrelator::FullCorrelator(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"full_sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"full_sequences", u"strings"});

        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 2;
    }



    void FullCorrelator::operator()(IOArgumentRange output, FullCorrelatorParams &input) {
        // Get stored moment matrix
        auto msPtr = input.matrix_system_key(this->storageManager);
        assert(msPtr); // ^- above should throw if absent
        auto& system = *msPtr;

        switch (input.export_shape) {
            case FullCorrelatorParams::ExportShape::WholeTensor:
                export_whole_tensor(output, input, system);
                return;
            case FullCorrelatorParams::ExportShape::OneCorrelator:
                export_one_correlator(output, input, system);
                return;
        }


    }

    void FullCorrelator::export_whole_tensor(IOArgumentRange output,
                                             FullCorrelatorParams &input, MatrixSystem &system) {
        auto* ptSystemPtr = dynamic_cast<MaintainsTensors*>(&system);
        if (nullptr == ptSystemPtr) {
            throw BadParameter{"MatrixSystem does not maintain a probability tensor."};
        }
        auto lock = ptSystemPtr->get_read_lock();
        ptSystemPtr->RefreshFullCorrelator(lock);
        const auto& full_correlator = ptSystemPtr->FullCorrelator();

        FullCorrelatorExporter exporter{this->matlabEngine, system};
        switch (input.output_mode) {
            case FullCorrelatorParams::OutputMode::OperatorSequences:
                output[0] = exporter.sequences(full_correlator);
                break;
            case FullCorrelatorParams::OutputMode::OperatorSequencesWithSymbolInfo:
                output[0] = exporter.sequences_with_symbols(full_correlator);
                break;
            case FullCorrelatorParams::OutputMode::Symbols:
                output[0] = exporter.symbols(full_correlator);
                break;
            default:
                throw InternalError{"Unknown output mode."};
        }
    }

    void FullCorrelator::export_one_correlator(IOArgumentRange output,
                                               FullCorrelatorParams &input, MatrixSystem& raw_system) {
        auto* ptSystemPtr = dynamic_cast<MaintainsTensors*>(&raw_system);
        if (nullptr == ptSystemPtr) {
            throw BadParameter{"MatrixSystem does not maintain a probability tensor."};
        }
        auto& system = *ptSystemPtr;

        auto lock = raw_system.get_read_lock();
        const auto& correlator = get_element(this->matlabEngine, input, system, lock);

        FullCorrelatorExporter exporter{this->matlabEngine, system};

        matlab::data::CellArray one_by_one = exporter.factory.createCellArray({1, 1});
        (*one_by_one.begin()) = [&](const MaintainsTensors& the_system) -> matlab::data::CellArray {

            switch (input.output_mode) {
                case FullCorrelatorParams::OutputMode::OperatorSequences: {
                    auto fullPolyInfo = exporter.sequence(correlator, the_system.CollinsGisin());
                    return fullPolyInfo.move_to_cell(exporter.factory);
                }
                case FullCorrelatorParams::OutputMode::OperatorSequencesWithSymbolInfo: {
                    auto fullPolyInfo = exporter.sequence_with_symbols(correlator, the_system.CollinsGisin());
                    return fullPolyInfo.move_to_cell(exporter.factory);
                }
                case FullCorrelatorParams::OutputMode::Symbols:
                    return exporter.symbol(correlator);
                default:
                    throw InternalError{"Unknown output mode."};
            }
        }(system);

        output[0] = std::move(one_by_one);

    }

}