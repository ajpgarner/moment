/**
 * new_inflation_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "new_inflation_matrix_system.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

#include "storage_manager.h"

#include <algorithm>
#include <sstream>

namespace Moment::mex::functions {
    namespace {
        std::unique_ptr<Inflation::InflationContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                                  NewInflationMatrixSystemParams &input) {
            return std::make_unique<Inflation::InflationContext>(
                    Inflation::CausalNetwork{input.outcomes_per_observable, std::move(input.source_init_list)},
                    input.inflation_level
                );
        }
    }

    NewInflationMatrixSystemParams::NewInflationMatrixSystemParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Either set named params OR give multiple params
        bool set_any_param  = this->params.contains(u"observables")
                              || this->params.contains(u"sources")
                              || this->params.contains(u"inflation_level");

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }
            this->getFromParams();
        } else {
            if (this->inputs.size() < 3) {
                throw errors::BadInput{errors::too_few_inputs,
                   "Input should be in the form: [outcomes per observable], [list of sources], inflation level."};
            }
            this->getFromInputs();
        }
    }

    void NewInflationMatrixSystemParams::getFromParams() {
        auto obsIter = this->params.find(u"observables");
        if (obsIter == this->params.cend()) {
            throw errors::BadInput{errors::too_few_inputs, "If parameters are set, \"observables\" should be set."};
        }

        auto sourceIter = this->params.find(u"sources");
        if (sourceIter == this->params.cend()) {
            throw errors::BadInput{errors::too_few_inputs, "If parameters are set, \"sources\" should be set."};
        }

        auto inflationIter = this->params.find(u"inflation_level");
        if (inflationIter == this->params.cend()) {
            throw errors::BadInput{errors::too_few_inputs, "If parameters are set, \"inflation\" should be set."};
        }

        this->outcomes_per_observable = read_positive_integer_array<size_t>(matlabEngine, "Parameter \"observables\"",
                                                                            obsIter->second, 0);

        readSourceCell(this->outcomes_per_observable.size(), sourceIter->second);

        this->inflation_level = read_positive_integer<size_t>(matlabEngine, "Parameter \"inflation_level\"",
                                                      inflationIter->second, 1);
    }

    void NewInflationMatrixSystemParams::getFromInputs() {
        this->outcomes_per_observable = read_positive_integer_array<size_t>(matlabEngine, "Observables",
                                                                            this->inputs[0], 0);
        readSourceCell(this->outcomes_per_observable.size(), this->inputs[1]);
        this->inflation_level = read_positive_integer<size_t>(matlabEngine, "Inflation level",
                                                              this->inputs[2], 1);
    }

    void NewInflationMatrixSystemParams::readSourceCell(const size_t num_observables,
                                                        const matlab::data::Array &input) {
        if (input.getType() != matlab::data::ArrayType::CELL) {
            throw errors::BadInput{errors::bad_param,
               "Source list should be provided as a cell array of arrays indicating connected observables."};
        }
        const matlab::data::CellArray cellInput = input;
        this->source_init_list.reserve(input.getNumberOfElements());
        for (const auto& cell : cellInput) {
            auto obsVec = read_positive_integer_array<uint64_t>(matlabEngine, "Observables", cell, 1);

            this->source_init_list.emplace_back();
            auto& targetSet = this->source_init_list.back();
            for (const auto& x : obsVec) {
                if (x > num_observables) {
                    std::stringstream errSS;
                    errSS << "Observable \"" << x << "\" out of bounds in source list.";
                    throw errors::BadInput{errors::bad_param, errSS.str()};
                }
                targetSet.emplace(x-1);
            }
        }

    }

    std::string NewInflationMatrixSystemParams::to_string() const {
        std::stringstream ss;
        size_t num_observables = this->outcomes_per_observable.size();
        size_t num_sources = this->source_init_list.size();

        ss << "New inflation matrix system with "
           << num_observables << ((1 != num_observables) ? " observables" : " observable")
           << " and "
           << num_sources << ((1 != num_sources) ? " sources" : " source") << ".\n";
        ss << "Inflation level: " << this->inflation_level << "\n";
        ss << "Outcomes per observable: ";
        bool one_o = false;
        for (auto o : this->outcomes_per_observable) {
            if (one_o) {
                ss << ", ";
            } else {
                one_o = true;
            }
            ss << o;
        }
        ss << "\n";

        ss << "Sources: \n";
        size_t s_index = 1;
        for (const auto& s : this->source_init_list) {
            ss << s_index << " -> ";
            bool one_o_in_s = false;
            for (auto o: s) {
                if (one_o_in_s) {
                    ss << ", ";
                } else {
                    one_o_in_s = true;
                }
                ss << (o+1);
            }
            ss << "\n";
            ++s_index;
        }

        return ss.str();
    }

    NewInflationMatrixSystem::NewInflationMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"new_inflation_matrix_system") {
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->min_inputs = 0;
        this->max_inputs = 3;

        this->param_names.emplace(u"inflation_level");
        this->param_names.emplace(u"observables");
        this->param_names.emplace(u"sources");
    }

    void NewInflationMatrixSystem::operator()(IOArgumentRange output, NewInflationMatrixSystemParams &input) {
        using namespace Inflation;

        // Interpret context
        std::unique_ptr<InflationContext> contextPtr = make_context(this->matlabEngine, input);

        // Output context in verbose mode
        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Make new system around context
        std::unique_ptr<MatrixSystem> matrixSystemPtr = std::make_unique<InflationMatrixSystem>(std::move(contextPtr));
        auto lock = matrixSystemPtr->get_read_lock();
        const auto& context = dynamic_cast<const InflationMatrixSystem*>(matrixSystemPtr.get())->InflationContext();

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);

        // Return list of canonical observable operator IDs
        if (output.size() > 1) {
            auto canonical_obs = factory.createArray<uint64_t>({1ULL, context.Observables().size()});
            auto write_iter = canonical_obs.begin();
            for (const auto& cObs : context.Observables()) {
                *write_iter = cObs.operator_offset;
                ++write_iter;
            }
            output[1] = std::move(canonical_obs);
        }
    }

}