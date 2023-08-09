/**
 * collins_gisin_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

#include "probability/collins_gisin.h"
#include "scenarios/locality/locality_collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/inflation/inflation_collins_gisin.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "storage_manager.h"

#include "export/export_collins_gisin.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "utilities/utf_conversion.h"

#include "mex.hpp"

#include <tuple>


namespace Moment::mex::functions  {

    namespace {
        [[nodiscard]] CollinsGisinRange get_slice(matlab::engine::MATLABEngine& matlabEngine,
                                                       const CollinsGisinParams& input,
                                                       MatrixSystem& system,
                                                       decltype(system.get_read_lock())& lock) {

            if (auto* lmsPtr = dynamic_cast<Locality::LocalityMatrixSystem*>(&system); lmsPtr != nullptr) {
                lmsPtr->RefreshCollinsGisin(lock);
                const auto& cg = lmsPtr->LocalityCollinsGisin();

                PMConvertor pmReader{matlabEngine, lmsPtr->localityContext, false};
                auto free_mmts = pmReader.read_pm_index_list(input.freeMeasurements);
                auto fixed_mmts = pmReader.read_pmo_index_list(input.fixedOutcomes);

                try {
                    return cg.measurement_to_range(free_mmts, fixed_mmts);
                } catch (const Moment::errors::BadCGError& cge) {
                    throw_error(matlabEngine, errors::bad_param, cge.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            if (auto* imsPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system); imsPtr != nullptr) {
                imsPtr->RefreshCollinsGisin(lock);
                const auto& cg = imsPtr->InflationCollinsGisin();

                OVConvertor ovReader{matlabEngine, imsPtr->InflationContext(), false};
                auto free_mmts = ovReader.read_ov_index_list(input.freeMeasurements);
                auto fixed_mmts = ovReader.read_ovo_index_list(input.fixedOutcomes);

                try {
                    return cg.measurement_to_range(free_mmts, fixed_mmts);
                } catch (const Moment::errors::BadCGError& cge) {
                    throw_error(matlabEngine, errors::bad_param, cge.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            throw_error(matlabEngine, errors::bad_param, "Matrix system must be a locality or inflation system.");
        }
    }


    CollinsGisinParams::CollinsGisinParams(SortedInputs &&inputIn)
            : SortedInputs(std::move(inputIn)) {

        // Get matrix system class
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // See if output type is set
        if (this->flags.contains(u"symbols")) {
            this->output_type = OutputType::SymbolIds;
        } else if (this->flags.contains(u"sequences")) {
            this->output_type = OutputType::Sequences;
        } else if (this->flags.contains(u"full_sequences")) {
            this->output_type = OutputType::SequencesWithSymbolInfo;
        } else if (this->flags.contains(u"strings")) {
            this->output_type = OutputType::SequenceStrings;
        }


        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_shape = ExportShape::WholeTensor;
            return;
        }

        // Otherwise, determine mode, and check dimensions
        if (this->inputs.size() == 2) {
            std::tie(this->freeMeasurements, this->fixedOutcomes)
                = read_pairs_and_triplets(this->matlabEngine, this->inputs[1]);
        } else {
            assert(this->inputs.size() == 3);
            std::tie(this->freeMeasurements, this->fixedOutcomes)
                = read_pairs_and_triplets(this->matlabEngine, this->inputs[1], this->inputs[2]);
        }
        this->export_shape = this->freeMeasurements.empty() ? ExportShape::OneOutcome : ExportShape::OneMeasurement;

    }

    CollinsGisin::CollinsGisin(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"full_sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"full_sequences", u"strings"});

        this->min_outputs = 1;
        this->max_outputs = 5;

        this->min_inputs = 1;
        this->max_inputs = 3;
    }


    void CollinsGisin::extra_input_checks(CollinsGisinParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }



    void CollinsGisin::operator()(IOArgumentRange output, CollinsGisinParams &input) {
        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent
        MatrixSystem& system = *msPtr;

        const bool allow_aliases = system.Context().can_have_aliases();

        // Check output count vs. processed input type.
        switch (input.output_type) {
            case CollinsGisinParams::OutputType::Sequences:
                if (output.size() != 2) {
                    throw_error(this->matlabEngine,
                                output.size() > 2 ? errors::too_many_outputs : errors::too_few_outputs,
                                "'sequences' mode expects two outputs [sequences, hashes].");
                }
                break;
            case CollinsGisinParams::OutputType::SequencesWithSymbolInfo:
                if ((output.size() != 4) && (output.size() != 5)) {

                    if (allow_aliases) {
                        throw_error(this->matlabEngine,
                                    output.size() > 5 ? errors::too_many_outputs : errors::too_few_outputs,
                                    "'full_sequences' mode expects five outputs [sequences, hashes, symbol ID, real basis elem, is aliased].");
                    } else {
                        throw_error(this->matlabEngine,
                                    output.size() > 4 ? errors::too_many_outputs : errors::too_few_outputs,
                                    "'full_sequences' mode expects four outputs [sequences, hashes, symbol ID, real basis elem].");
                    }
                }
                break;
            case CollinsGisinParams::OutputType::SymbolIds:
                if ((output.size() != 2) && (output.size() != 3)) {
                    if (allow_aliases) {
                        throw_error(this->matlabEngine,
                                    output.size() > 3 ? errors::too_many_outputs : errors::too_few_outputs,
                                    "'symbols' mode expects two outputs [symbol IDs, basis elements, alias status].");
                    } else {
                        throw_error(this->matlabEngine,
                                    output.size() > 2 ? errors::too_many_outputs : errors::too_few_outputs,
                                    "'symbols' mode expects two outputs [symbol IDs, basis elements].");
                    }
                }
                break;
            case CollinsGisinParams::OutputType::SequenceStrings:
                if (output.size() != 1) {
                    throw_error(this->matlabEngine,
                                output.size() > 1 ? errors::too_many_outputs : errors::too_few_outputs,
                                "'strings' mode expects one output.");
                }
                break;
        }


        switch (input.export_shape) {
            case CollinsGisinParams::ExportShape::WholeTensor:
                export_whole_tensor(output, input, system);
                break;
            case CollinsGisinParams::ExportShape::OneMeasurement:
                export_one_measurement(output, input, system);
                break;
            case CollinsGisinParams::ExportShape::OneOutcome:
                export_one_outcome(output, input, system);
                break;
        }

    }

    void CollinsGisin::export_whole_tensor(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system) {
        // Get read lock
        auto lock = system.get_read_lock();
        const bool can_have_alias = system.Context().can_have_aliases();

        const auto& cg = [&]() -> const Moment::CollinsGisin& {
            try {
                auto* mtPtr = dynamic_cast<MaintainsTensors*>(&system);
                if (nullptr == mtPtr) {
                    throw_error(this->matlabEngine, errors::bad_param,
                                "Matrix system must be a locality or inflation system.");
                }
                mtPtr->RefreshCollinsGisin(lock);
                return mtPtr->CollinsGisin();
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(this->matlabEngine, errors::missing_cg, mce.what());
            } catch (const Moment::errors::BadCGError& cge) {
                throw_error(this->matlabEngine, errors::missing_cg, cge.what());
            }
        }();

        CollinsGisinExporter cge{this->matlabEngine, system.Context(), system.Symbols()};
        try {
            switch (input.output_type) {
                case CollinsGisinParams::OutputType::SymbolIds:
                    if (can_have_alias && (output.size() >= 3)) {
                        std::tie(output[0], output[1], output[2]) = cge.symbol_basis_and_alias(cg);
                    } else {
                        std::tie(output[0], output[1]) = cge.symbol_and_basis(cg);
                        if (output.size() > 2) {
                            output[2] = cge.factory.createEmptyArray();
                        }
                    }
                    break;
                case CollinsGisinParams::OutputType::Sequences:
                    std::tie(output[0], output[1]) = cge.sequence_and_hash(cg);
                    break;
                case CollinsGisinParams::OutputType::SequencesWithSymbolInfo:
                    if (can_have_alias && (output.size() > 4)) {
                        std::tie(output[0], output[1], output[2], output[3], output[4])
                                = cge.everything_with_aliases(cg);
                    } else {
                        std::tie(output[0], output[1], output[2], output[3]) = cge.everything(cg);
                        if (output.size() > 4) {
                            output[4] = cge.factory.createEmptyArray();
                        }
                    }

                     break;
                case CollinsGisinParams::OutputType::SequenceStrings: {
                    auto formatter = this->settings->get_locality_formatter();
                    assert(formatter);
                    output[0] = cge.strings(cg, *formatter);
                }
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
            }
        } catch (const Moment::errors::BadCGError& cge) {
            throw_error(this->matlabEngine, errors::missing_cg, cge.what());
        }
    }

    void CollinsGisin::export_one_measurement(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system) {
        auto lock = system.get_read_lock();
        const bool can_have_alias = system.Context().can_have_aliases();
        auto slice = get_slice(this->matlabEngine, input, system, lock);

        CollinsGisinExporter cge{this->matlabEngine, system.Context(), system.Symbols()};
        try {
            switch (input.output_type) {
                case CollinsGisinParams::OutputType::SymbolIds:
                    if (can_have_alias && (output.size() >= 3)) {
                        std::tie(output[0], output[1], output[2]) = cge.symbol_basis_and_alias(slice);
                    } else {
                        std::tie(output[0], output[1]) = cge.symbol_and_basis(slice);
                        if (output.size() > 2) {
                            output[2] = cge.factory.createEmptyArray();
                        }
                    }
                    break;
                case CollinsGisinParams::OutputType::Sequences:
                    std::tie(output[0], output[1]) = cge.sequence_and_hash(slice);
                    break;
                case CollinsGisinParams::OutputType::SequencesWithSymbolInfo:
                    if (can_have_alias && (output.size() > 4)) {
                        std::tie(output[0], output[1], output[2], output[3], output[4])
                                = cge.everything_with_aliases(slice);
                    } else {
                        std::tie(output[0], output[1], output[2], output[3]) = cge.everything(slice);
                        if (output.size() > 4) {
                            output[4] = cge.factory.createEmptyArray();
                        }
                    }
                    break;
                case CollinsGisinParams::OutputType::SequenceStrings: {
                    auto formatter = this->settings->get_locality_formatter();
                    assert(formatter);
                    output[0] = cge.strings(slice, *formatter);
                }
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
            }
        } catch (const Moment::errors::BadCGError& cge) {
            throw_error(this->matlabEngine, errors::missing_cg, cge.what());
        }
    }

    void CollinsGisin::export_one_outcome(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system) {
        auto lock = system.get_read_lock();
        const bool can_have_alias = system.Context().can_have_aliases();

        auto slice = get_slice(this->matlabEngine, input, system, lock);

        // Check there is one element referred to.
        auto iter = slice.begin();
        if (iter == slice.end()) {
            throw_error(matlabEngine, errors::internal_error, "Invalid measurement.");
        }

        CollinsGisinExporter cge{this->matlabEngine, system.Context(), system.Symbols()};
        try {
            switch (input.output_type) {
                case CollinsGisinParams::OutputType::SymbolIds:
                    if (can_have_alias && (output.size() >= 3)) {
                        std::tie(output[0], output[1], output[2]) = cge.symbol_basis_and_alias(slice);
                    } else {
                        std::tie(output[0], output[1]) = cge.symbol_and_basis(slice);
                        if (output.size() > 2) {
                            output[2] = cge.factory.createEmptyArray();
                        }
                    }
                    break;
                case CollinsGisinParams::OutputType::Sequences:
                    std::tie(output[0], output[1]) = cge.sequence_and_hash(slice);
                    break;
                case CollinsGisinParams::OutputType::SequencesWithSymbolInfo:
                    if (can_have_alias && (output.size() > 4)) {
                        std::tie(output[0], output[1], output[2], output[3], output[4])
                            = cge.everything_with_aliases(slice);
                    } else {
                        std::tie(output[0], output[1], output[2], output[3]) = cge.everything(slice);
                        if (output.size() > 4) {
                            output[4] = cge.factory.createEmptyArray();
                        }
                    }
                    break;
                case CollinsGisinParams::OutputType::SequenceStrings: {
                    auto formatter = this->settings->get_locality_formatter();
                    assert(formatter);
                    output[0] = cge.strings(slice, *formatter);
                }
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
            }
        } catch (const Moment::errors::BadCGError& cge) {
            throw_error(this->matlabEngine, errors::missing_cg, cge.what());
        }
    }
}