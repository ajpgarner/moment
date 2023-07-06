/**
 * collins_gisin_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

#include "probability/collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "storage_manager.h"

#include "export/export_collins_gisin.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "utilities/utf_conversion.h"

#include "mex.hpp"


namespace Moment::mex::functions  {

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
    }

    CollinsGisin::CollinsGisin(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"full_sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"full_sequences", u"strings"});

        this->min_outputs = 1;
        this->max_outputs = 4;

        this->min_inputs = 1;
        this->max_inputs = 1;
    }


    void CollinsGisin::extra_input_checks(CollinsGisinParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }



    void CollinsGisin::operator()(IOArgumentRange output, CollinsGisinParams &input) {
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
                if (output.size() != 4) {
                    throw_error(this->matlabEngine,
                                output.size() > 4 ? errors::too_many_outputs : errors::too_few_outputs,
                                "'full_sequences' mode expects four outputs [sequences, hashes, symbol ID, real basis elem].");
                }
                break;
            case CollinsGisinParams::OutputType::SymbolIds:
                if (output.size() != 2) {
                    throw_error(this->matlabEngine,
                                output.size() > 2 ? errors::too_many_outputs : errors::too_few_outputs,
                                "'symbols' mode expects two outputs [symbol IDs, basis elements].");
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
                export_whole_tensor(output, input);
                break;
            case CollinsGisinParams::ExportShape::OneMeasurement:
                export_one_measurement(output, input);
                break;
            case CollinsGisinParams::ExportShape::OneOutcome:
                export_one_outcome(output, input);
                break;
        }

    }

    void CollinsGisin::export_whole_tensor(IOArgumentRange output, CollinsGisinParams &input) {

        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent

        // Get read lock
        MatrixSystem& system = *msPtr;
        auto lock = msPtr->get_read_lock();

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
                throw_error(this->matlabEngine, "missing_cg", mce.what());
            } catch (const Moment::errors::BadCGError& cge) {
                throw_error(this->matlabEngine, "missing_cg", cge.what());
            }
        }();

        CollinsGisinExporter cge{this->matlabEngine, system.Context(), system.Symbols()};
        try {
            switch (input.output_type) {
                case CollinsGisinParams::OutputType::SymbolIds: {
                    auto [symbols, bases] = cge.symbol_and_basis(cg);
                    output[0] = std::move(symbols);
                    output[1] = std::move(bases);
                }    break;
                case CollinsGisinParams::OutputType::Sequences: {
                    auto [sequences, hashes] = cge.sequence_and_hash(cg);
                    output[0] = std::move(sequences);
                    output[1] = std::move(hashes);
                } break;
                case CollinsGisinParams::OutputType::SequencesWithSymbolInfo: {
                    auto [sequences, hashes, symbols, bases] = cge.everything(cg);
                    output[0] = std::move(sequences);
                    output[1] = std::move(hashes);
                    output[2] = std::move(symbols);
                    output[3] = std::move(bases);
                } break;
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
            throw_error(this->matlabEngine, "missing_cg", cge.what());
        }
    }

    void CollinsGisin::export_one_measurement(IOArgumentRange output, CollinsGisinParams &input) {
        throw_error(this->matlabEngine, errors::internal_error,
                    "CollinsGisin::export_one_measurement not implemented.");
    }

    void CollinsGisin::export_one_outcome(IOArgumentRange output, CollinsGisinParams &input) {
        throw_error(this->matlabEngine, errors::internal_error,
                    "CollinsGisin::export_one_outcome not implemented.");

    }
}