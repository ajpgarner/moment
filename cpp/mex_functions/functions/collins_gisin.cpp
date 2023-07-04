/**
 * collins_gisin_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

#include "scenarios/collins_gisin.h"
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
            this->outputType = OutputType::SymbolIds;
        } else if (this->flags.contains(u"sequences")) {
            this->outputType = OutputType::Sequences;
        } else if (this->flags.contains(u"strings")) {
            this->outputType = OutputType::SequenceStrings;
        }
    }

    CollinsGisin::CollinsGisin(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"strings"});

        this->min_outputs = 1;
        this->max_outputs = 2;

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
        switch (input.outputType) {
            case CollinsGisinParams::OutputType::Sequences:
                if (output.size() != 2) {
                    throw_error(this->matlabEngine,
                                output.size() > 2 ? errors::too_many_outputs : errors::too_few_outputs,
                                "'sequences' mode expects two outputs [sequences, hashes].");
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

        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent

        // Get read lock
        auto lock = msPtr->get_read_lock();
        MatrixSystem& system = *msPtr;

        // Create (or retrieve) CG information
        try {

            /* Get CG tensor */
            const auto& cg = [&]() -> const Moment::CollinsGisin& {
                 auto *lms = dynamic_cast<Locality::LocalityMatrixSystem *>(&system);
                if (nullptr != lms) {
                    lms->RefreshCollinsGisin(lock);
                    return lms->CollinsGisin();
                } else {
                    auto *ims = dynamic_cast<Inflation::InflationMatrixSystem*>(&system);
                    if (nullptr != ims) {
                        ims->RefreshCollinsGisin(lock);
                        return ims->CollinsGisin();
                    }
                    throw_error(this->matlabEngine, errors::bad_param,
                                "Matrix system must be a locality or inflation system.");
                }
            }();

            CollinsGisinExporter cge{this->matlabEngine, system.Context(), system.Symbols()};



            // Export whole matrix?
            if (output.size() >= 1) {
                switch (input.outputType) {
                    case CollinsGisinParams::OutputType::SymbolIds:
                        try {
                            auto [symbols, bases] = cge.symbol_and_basis(cg);
                            output[0] = std::move(symbols);
                            output[1] = std::move(bases);
                        } catch (const Moment::errors::BadCGError& bcge) {
                            throw_error(this->matlabEngine, "missing_cg", bcge.what());
                        }
                        break;
                    case CollinsGisinParams::OutputType::Sequences: {
                        auto [sequences, hashes] = cge.sequence_and_hash(cg);
                        output[0] = std::move(sequences);
                        output[1] = std::move(hashes);
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

            }
        } catch (const Moment::errors::missing_component& mce) {
            throw_error(this->matlabEngine, "missing_cg", mce.what());
        } catch (const Moment::errors::BadCGError& cge) {
            throw_error(this->matlabEngine, "missing_cg", cge.what());
        }
    }
}