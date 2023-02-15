/**
 * make_explicit.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "make_explicit.h"

#include "storage_manager.h"

#include "export/export_substitution_list.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_implicit_symbols.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_implicit_symbols.h"

#include <cmath>
#include <sstream>

namespace Moment::mex::functions {

    namespace {
        std::vector<Inflation::OVIndex> mmts_to_ov_index(matlab::engine::MATLABEngine &matlabEngine,
                                                         const Inflation::InflationContext& context,
                                                         MakeExplicitParams& input) {
            const size_t num_obs = context.Observables().size();
            std::vector<Inflation::OVIndex> output;
            if (input.input_type == MakeExplicitParams::InputType::AllMeasurements) {

                output.reserve(num_obs);
                for (size_t o = 0; o < num_obs; ++o) {
                    output.emplace_back(o, 0);
                }
            } else {
                output.reserve(input.measurements_or_observables.size());
                for (auto [observable, variant] : input.measurements_or_observables) {
                    if ((observable > num_obs) || (observable <= 0)) {
                        std::stringstream errSS;
                        errSS << "Observable " << observable << " out of range.";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }
                    const auto& obsInfo = context.Observables()[observable-1];
                    if ((variant > obsInfo.variant_count) || (variant <= 0)) {
                        std::stringstream errSS;
                        errSS << "Variant " << variant << " out of range for observable " << observable << ".";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }

                    output.emplace_back(observable - 1, variant - 1);
                }
            }

            return output;
        }


        std::vector<Locality::PMIndex> mmts_to_pm_index(matlab::engine::MATLABEngine &matlabEngine,
                                                         const Locality::LocalityContext& context,
                                                         MakeExplicitParams& input) {
            const size_t num_parties = context.Parties.size();
            std::vector<Locality::PMIndex> output;
            if (input.input_type == MakeExplicitParams::InputType::AllMeasurements) {
                output.reserve(num_parties);
                for (size_t o = 0; o < num_parties; ++o) {
                    output.emplace_back(o, 0);
                }
            } else {
                output.reserve(input.measurements_or_observables.size());
                for (auto [party, measurement] : input.measurements_or_observables) {
                    if ((party > num_parties) || (party <= 0)) {
                        std::stringstream errSS;
                        errSS << "Party " << party << " out of range.";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }
                    const auto& partyInfo = context.Parties[party-1];
                    const size_t num_mmts = partyInfo.Measurements.size();
                    if ((measurement > num_mmts ) || (measurement <= 0)) {
                        std::stringstream errSS;
                        errSS << "Measurement " << measurement << " out of range for party " << party << ".";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }

                    output.emplace_back(party - 1, measurement - 1);
                }
            }
            context.populate_global_mmt_index(output);
            return output;
        }


    }

    MakeExplicitParams::MakeExplicitParams(SortedInputs &&structuredInputs)
           : SortedInputs(std::move(structuredInputs)) {
        // Get system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        size_t value_input_index = 2;
        if (this->inputs.size() >= 3) {
            this->input_type = InputType::SpecifiedMeasurement;

            // Try to read measurement information
            auto inputTwoDims = this->inputs[1].getDimensions();
            if (inputTwoDims.size() != 2) {
                throw_error(matlabEngine, errors::bad_param,
                            "Measurement/observable list must be a vector or Nx2 matrix.");
            }
            if (inputTwoDims[1] == 1) {
                auto matLine = read_as_vector<uint64_t>(matlabEngine, this->inputs[1]);
                this->measurements_or_observables.reserve(matLine.size());
                for (auto x : matLine) {
                    this->measurements_or_observables.emplace_back(x, 1ULL);
                }
            } else if (inputTwoDims[1] == 2) {
                this->measurements_or_observables.reserve(inputTwoDims[0]);
                for (size_t row = 0; row < inputTwoDims[0]; ++row) {
                    this->measurements_or_observables.emplace_back(
                            static_cast<uint64_t>(this->inputs[1][row][0]),
                            static_cast<uint64_t>(this->inputs[1][row][1]));

                }
            } else {
                throw_error(matlabEngine, errors::bad_param,
                            "Measurement/observable list must be a vector or Nx2 matrix.");
            }
        } else {
            value_input_index = 1;
            this->input_type = InputType::AllMeasurements;
        }

        this->values = read_as_vector<double>(matlabEngine, this->inputs[value_input_index]);

    }


    MakeExplicit::MakeExplicit(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction(matlabEngine, storage, u"make_explicit") {
        this->min_inputs = 2;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void MakeExplicit::extra_input_checks(MakeExplicitParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    void MakeExplicit::operator()(IOArgumentRange output, MakeExplicitParams &input) {
        // Get matrix system ptr from storage
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        // Lock for changes
        auto read_lock = matrixSystem.get_read_lock();

        // Can we read as a locality scenario?
        const auto* lmsPtr = dynamic_cast<const Locality::LocalityMatrixSystem*>(&matrixSystem);
        if (lmsPtr != nullptr) {
            output[0] = this->do_make_explicit(*lmsPtr, input);
            return;
        }

        // Can we read as an inflation scenario?
        const auto* imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&matrixSystem);
        if (imsPtr != nullptr) {
            output[0] = this->do_make_explicit(*imsPtr, input);
            return;
        }

        throw_error(this->matlabEngine, errors::bad_param,
                    "Supplied matrix system must be either a locality or an inflation matrix system.");
    }

    matlab::data::Array
    MakeExplicit::do_make_explicit(const Inflation::InflationMatrixSystem &ims, MakeExplicitParams &input) {
        auto ov_indices = mmts_to_ov_index(this->matlabEngine, ims.InflationContext(), input);
        const auto& is_table = ims.ImplicitSymbolTable();

        try {
            auto explicit_form = is_table.implicit_to_explicit(ov_indices, input.values);

            auto unit_value = explicit_form.find(1);
            if (unit_value != explicit_form.end()) {
                if ((!this->quiet) && (std::abs(unit_value->second - 1.0) > 1e-7)) {
                    std::stringstream warningSS;
                    warningSS << "WARNING: probability distribution supplied summed up to " << unit_value->second
                              << " but unity was expected.\n";
                    print_to_console(this->matlabEngine, warningSS.str());
                }
                explicit_form.erase(unit_value);
            }
            return export_substitution_list(matlabEngine, explicit_form);
        } catch (const Moment::errors::implicit_to_explicit_error& itee) {
            throw_error(this->matlabEngine, errors::bad_param, itee.what());
        }
    }

    matlab::data::Array
    MakeExplicit::do_make_explicit(const Locality::LocalityMatrixSystem &lms, MakeExplicitParams &input) {
        auto pm_indices = mmts_to_pm_index(this->matlabEngine, lms.localityContext, input);
        const auto& is_table = lms.ImplicitSymbolTable();
        try {
            auto explicit_form = is_table.implicit_to_explicit(pm_indices, input.values);

            auto unit_value = explicit_form.find(1);
            if (unit_value != explicit_form.end()) {
                if ((!this->quiet) && (std::abs(unit_value->second - 1.0) > 1e-7)) {
                    std::stringstream warningSS;
                    warningSS << "WARNING: probability distribution supplied summed up to " << unit_value->second
                              << " but unity was expected.\n";
                    print_to_console(this->matlabEngine, warningSS.str());
                }
                explicit_form.erase(unit_value);
            }
            return export_substitution_list(matlabEngine, explicit_form);
        } catch (const Moment::errors::implicit_to_explicit_error& itee) {
            throw_error(this->matlabEngine, errors::bad_param, itee.what());
        }
    }
}