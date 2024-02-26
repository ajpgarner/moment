/**
 * locality_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_matrix_system.h"

#include "storage_manager.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"
#include "utilities/persistent_storage.h"

#include <numeric>

namespace Moment::mex::functions {
    namespace {
        std::unique_ptr<Locality::LocalityContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                                const LocalityMatrixSystemParams &input) {
            return std::make_unique<Locality::LocalityContext>(
                    Locality::Party::MakeList(input.mmts_per_party, input.outcomes_per_mmt)
            );
        }
    }

    LocalityMatrixSystemParams::LocalityMatrixSystemParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Either set named params OR give multiple params
        bool set_any_param  = this->params.contains(u"parties")
                                  || this->params.contains(u"measurements")
                                  || this->params.contains(u"outcomes");

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw BadParameter{"Input arguments should be exclusively named, or exclusively unnamed."};
            }
            this->getFromParams();
        } else {
            // No named parameters... try to interpret inputs as Settings object + depth
            // Otherwise, try to interpret inputs as flat specification
            this->getFromInputs();
        }

        // Optional parameters
        auto tolerance_param_iter = this->params.find(u"tolerance");
        if (tolerance_param_iter != this->params.cend()) {
            this->zero_tolerance = read_as_double(this->matlabEngine, tolerance_param_iter->second);
            if (this->zero_tolerance < 0) {
                throw BadParameter{"Tolerance must be non-negative value."};
            }
        }
    }

    void LocalityMatrixSystemParams::getFromParams() {
        // Read and check number of parties [default: 1 party]
        if (!find_and_parse(u"parties", [this](matlab::data::Array& parties) {
            this->number_of_parties = read_positive_integer<size_t>(matlabEngine, "Parameter 'parties'",
                                                                    parties, 1);
        })) {
            this->number_of_parties = 1;
        }

        // Read and check measurements [default: 1 per party]
        this->mmts_per_party.reserve(number_of_parties);

        if (!find_and_parse(u"measurements", [this](matlab::data::Array& mmts) {
            this->readMeasurementSpecification(mmts, "Parameter 'measurements'");
        })) {
            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, 1);
            this->total_measurements = number_of_parties;
        }

        // Read outcomes per measurement (MUST be specified)
        find_and_parse_or_throw(u"outcomes", [this](matlab::data::Array& outcomes) {
            this->readOutcomeSpecification(outcomes, "Parameter 'outcomes'");
        });
    }


    void LocalityMatrixSystemParams::getFromInputs() {
        if (inputs.size() < 2) {
            std::string errStr{"Please supply either named inputs; or a list of integers in the form"};
            errStr += " \"number of parties, number of outcomes\",";
            errStr += " or \"number of parties, measurements per party, outcomes per measurement\".";
            throw InputCountException{"locality_matrix_system", 2, 3, inputs.size(),
                                      errStr};
        }

        // Get number of parties
        this->number_of_parties = read_positive_integer<size_t>(matlabEngine, "Party count",
                                                                inputs[0], 1);

        // Read measurements (if any) and operator count
        if (inputs.size() == 3) {
            this->readMeasurementSpecification(inputs[1], "Measurement count");
            this->readOutcomeSpecification(inputs[2], "Number of outcomes");
        } else {
            // Auto 1 mmt per party
            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, 1);
            this->total_measurements = 1;
            this->readOutcomeSpecification(inputs[1], "Number of outcomes");
        }
    }

    void LocalityMatrixSystemParams::readMeasurementSpecification(matlab::data::Array &input,
                                                                  const std::string& paramName) {

        const size_t num_elems = input.getNumberOfElements();
        if (1 == num_elems) {
            size_t flat_mmts_per_party = read_positive_integer<size_t>(matlabEngine, paramName, input, 1);

            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, flat_mmts_per_party);
            this->total_measurements = this->number_of_parties * flat_mmts_per_party;
        } else if (number_of_parties == num_elems) {
            this->mmts_per_party = read_positive_integer_array<size_t>(matlabEngine, paramName, input, 1);
            this->total_measurements = std::accumulate(mmts_per_party.cbegin(), mmts_per_party.cend(),
                                                       static_cast<uint64_t>(0));
        } else {
            throw BadParameter{paramName + " should either be a scalar, or an array with one value per party."};
        }

    }

    void LocalityMatrixSystemParams::readOutcomeSpecification(matlab::data::Array &input,
                                                              const std::string& paramName) {

        const size_t num_elems = input.getNumberOfElements();
        if (1 == num_elems) {
            size_t flat_outcomes_per_mmt = read_positive_integer<size_t>(matlabEngine, paramName, input, 1);
            std::fill_n(std::back_inserter(this->outcomes_per_mmt),
                        this->total_measurements, flat_outcomes_per_mmt);

        } else if (this->total_measurements == num_elems) {
            this->outcomes_per_mmt = read_positive_integer_array<size_t>(matlabEngine, paramName, input, 1);
        } else {
            throw BadParameter{paramName + " should either be a scalar, or an array with one value per measurement."};
        }

        // Count operators...
        this->total_operators = 0;
        for (auto opm : outcomes_per_mmt) {
            this->total_operators += (opm - 1);
        }
    }

    LocalityMatrixSystem::LocalityMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"parties");
        this->param_names.emplace(u"measurements");
        this->param_names.emplace(u"outcomes");

        this->param_names.emplace(u"tolerance");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }

    void LocalityMatrixSystem::operator()(IOArgumentRange output, LocalityMatrixSystemParams &input) {
        // Input to context:
        std::unique_ptr<Locality::LocalityContext> contextPtr{make_context(this->matlabEngine, input)};
        if (!contextPtr) {
            throw InternalError{"Context object could not be created."};
        }

        // Output context in verbose mode
        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Make new system around context
        std::unique_ptr<MatrixSystem> matrixSystemPtr
            = std::make_unique<Locality::LocalityMatrixSystem>(std::move(contextPtr), input.zero_tolerance);

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }
}