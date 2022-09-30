/**
 * new_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "new_matrix_system.h"

#include "storage_manager.h"

#include "operators/context.h"
#include "operators/matrix/matrix_system.h"
#include "operators/locality/locality_context.h"
#include "operators/locality/locality_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"
#include "utilities/persistent_storage.h"

#include <numeric>

namespace NPATK::mex::functions {
    namespace {
        std::unique_ptr<Context> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                              const NewMatrixSystemParams &input) {
            switch (input.system_type) {
                case NewMatrixSystemParams::SystemType::Generic:
                    return std::make_unique<Context>(input.total_operators);
                case NewMatrixSystemParams::SystemType::Locality:
                    return std::make_unique<LocalityContext>(
                            Party::MakeList(input.mmts_per_party, input.outcomes_per_mmt));
            }
            throw_error(matlabEngine, errors::internal_error, "Could not make context: Unknown scenario type.");
        }
    }

    NewMatrixSystemParams::NewMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Either set named params OR give multiple params
        bool set_any_locality_params  = this->params.contains(u"parties")
                                  || this->params.contains(u"measurements")
                                  || this->params.contains(u"outcomes");

        bool set_any_generic_params = this->params.contains(u"operators");

        bool set_any_param = set_any_locality_params || set_any_generic_params;

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            if (set_any_locality_params) {
                this->getLocalityFromParams(matlabEngine);
            } else {
                this->getGenericFromParams(matlabEngine);
            }

        } else {

            // No named parameters... try to interpret inputs as Settings object + depth
            // Otherwise, try to interpret inputs as flat specification
            this->getFromInputs(matlabEngine);
        }
    }

    void NewMatrixSystemParams::getGenericFromParams(matlab::engine::MATLABEngine &matlabEngine) {
        // Read number of operators
        auto oper_param = params.find(u"operators");
        if (oper_param == params.end()) {
            throw errors::BadInput{errors::too_few_inputs, "Missing \"operators\" parameter."};
        }
        this->readOperatorSpecification(matlabEngine, oper_param->second, "Parameter 'operators'");
        this->system_type = SystemType::Generic;
    }

    void NewMatrixSystemParams::getLocalityFromParams(matlab::engine::MATLABEngine &matlabEngine) {
        // Read and check number of parties, or default to 1
        auto party_param = params.find(u"parties");
        if (party_param != params.end()) {
            this->number_of_parties = read_positive_integer(matlabEngine, "Parameter 'parties'",
                                                      party_param->second, 1);
        } else {
            this->number_of_parties = 1;
        }

        // Read and check measurements [default: 1 per party]
        this->mmts_per_party.reserve(number_of_parties);
        auto mmt_param = params.find(u"measurements");
        if (mmt_param != params.end()) {
            this->readMeasurementSpecification(matlabEngine, mmt_param->second, "Parameter 'measurements'");
        } else {
            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, 1);
            this->total_measurements = number_of_parties;
        }

        // Read outcomes per measurement
        auto outcome_param = params.find(u"outcomes");
        if (outcome_param == params.end()) {
            throw errors::BadInput{errors::missing_param,
                                   "Parameter 'outcomes' must be set."};
        }
        this->readOutcomeSpecification(matlabEngine, outcome_param->second, "Parameter 'outcomes'");

        this->system_type = SystemType::Locality;
    }


    void NewMatrixSystemParams::getFromInputs(matlab::engine::MATLABEngine &matlabEngine) {
        if (inputs.empty()) {
            std::string errStr{"Please supply either named inputs; or a list of integers in the"};
            errStr += " form of \"operators\", ";
            errStr += " '\"parties, number of outcomes\", ";
            errStr += "or \"parties, measurements per party, outcomes per measurement\".";
            throw errors::BadInput{errors::too_few_inputs, errStr};
        }

        // Generic system?
        if (inputs.size() == 1) {
            // Operator_index stays as 0: op
            this->system_type = SystemType::Generic;
            this->readOperatorSpecification(matlabEngine, inputs[0], "Number of operators");
            return;
        }

        // Otherwise, locality system
        this->system_type = SystemType::Locality;

        // Get number of parties
        if (this->system_type == SystemType::Locality) {
            if (inputs.size() >= 2) { // 2 or 3 inputs
                this->number_of_parties = read_positive_integer(matlabEngine, "Party count",
                                                          inputs[0], 1);
            } else {
                this->number_of_parties = 1;
            }
        }

        // Read measurements (if any) and operator count
        if (inputs.size() == 3) {
            this->readMeasurementSpecification(matlabEngine, inputs[1], "Measurement count");
            this->readOutcomeSpecification(matlabEngine, inputs[2], "Number of outcomes");
        } else {
            // Auto 1 mmt per party
            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, 1);
            this->total_measurements = 1;
            this->readOutcomeSpecification(matlabEngine, inputs[1], "Number of outcomes");
        }
    }

    void NewMatrixSystemParams::readMeasurementSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                             matlab::data::Array &input,
                                                             const std::string& paramName) {

        const size_t num_elems = input.getNumberOfElements();
        if (1 == num_elems) {
            size_t flat_mmts_per_party = read_positive_integer(matlabEngine, paramName,
                                                               input, 1);

            std::fill_n(std::back_inserter(this->mmts_per_party), number_of_parties, flat_mmts_per_party);
            this->total_measurements = this->number_of_parties * flat_mmts_per_party;
        } else if (number_of_parties == num_elems) {
            this->mmts_per_party = read_positive_integer_array(matlabEngine, paramName,
                                                               input, 1);
            this->total_measurements = std::accumulate(mmts_per_party.cbegin(), mmts_per_party.cend(),
                                                       static_cast<uint64_t>(0));
        } else {
            throw errors::BadInput{errors::bad_param,
                                   paramName + " should either be a scalar, or an array with one value per party."};
        }

    }

    void NewMatrixSystemParams::readOutcomeSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                         matlab::data::Array &input,
                                                         const std::string& paramName) {

        const size_t num_elems = input.getNumberOfElements();
        if (1 == num_elems) {
            size_t flat_outcomes_per_mmt = read_positive_integer(matlabEngine, paramName, input, 1);
            std::fill_n(std::back_inserter(this->outcomes_per_mmt),
                        this->total_measurements, flat_outcomes_per_mmt);

        } else if (this->total_measurements == num_elems) {
            this->outcomes_per_mmt = read_positive_integer_array(matlabEngine, paramName, input, 1);
        } else {
            throw errors::BadInput{errors::bad_param,
               paramName + " should either be a scalar, or an array with one value per measurement."};
        }

        // Count operators...
        this->total_operators = 0;
        for (auto opm : outcomes_per_mmt) {
            this->total_operators += (opm - 1);
        }
    }

    void NewMatrixSystemParams::readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                          matlab::data::Array &input,
                                                          const std::string& paramName) {
        this->total_operators = read_positive_integer(matlabEngine, paramName, input, 1);
    }


    std::unique_ptr<SortedInputs>
    NewMatrixSystem::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        return std::make_unique<NewMatrixSystemParams>(this->matlabEngine, std::move(input));
    }



    NewMatrixSystem::NewMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::NewMatrixSystem, u"new_matrix_system") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"parties");
        this->param_names.emplace(u"measurements");
        this->param_names.emplace(u"outcomes");
        this->param_names.emplace(u"operators");

        // One of three ways to input:
        this->mutex_params.add_mutex(u"outcomes", u"operators");

        this->min_inputs = 0;
        this->max_inputs = 3;
    }


    void NewMatrixSystem::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<NewMatrixSystemParams&>(*inputPtr);

        // Input to context:
        std::unique_ptr<Context> contextPtr{make_context(this->matlabEngine, input)};
        if (!contextPtr) {
            throw_error(this->matlabEngine, errors::internal_error, "Context object could not be created.");
        }

        // Output context in verbose mode
        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Make new system around context
        std::unique_ptr<MatrixSystem> matrixSystemPtr;
        if (input.system_type == NewMatrixSystemParams::SystemType::Generic) {
            matrixSystemPtr = std::make_unique<MatrixSystem>(std::move(contextPtr));
        } else if (input.system_type == NewMatrixSystemParams::SystemType::Locality) {
            matrixSystemPtr = std::make_unique<LocalityMatrixSystem>(std::move(contextPtr));
        }


        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }
}