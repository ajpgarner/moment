/**
 * make_moment_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_moment_matrix.h"

#include "operators/context.h"
#include "operators/moment_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include <memory>

namespace NPATK::mex::functions {

    namespace {
        std::unique_ptr<Context> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                              const MakeMomentMatrixParams& input) {
            std::vector<PartyInfo> party_list{};

            switch (input.specification_mode) {
                case MakeMomentMatrixParams::SpecificationMode::Unknown:
                    break;
                case MakeMomentMatrixParams::SpecificationMode::FlatNoMeasurements:
                    party_list = PartyInfo::MakeList(input.number_of_parties,
                                                     input.flat_operators_per_party);
                    break;
                case MakeMomentMatrixParams::SpecificationMode::FlatWithMeasurements:
                    party_list = PartyInfo::MakeList(input.number_of_parties,
                                                     input.flat_mmts_per_party, input.flat_outcomes_per_mmt);
                    break;
                case MakeMomentMatrixParams::SpecificationMode::PartyListOfOperators:
                case MakeMomentMatrixParams::SpecificationMode::PartyListOfMeasurements:
                    throw_error(matlabEngine, "not_implemented", "Not implemented.");
            }

            return std::make_unique<Context>(std::move(party_list));
        }
    }

    MakeMomentMatrix::MakeMomentMatrix(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeMomentMatrix, u"make_moment_matrix") {
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"sequences");
        this->param_names.emplace(u"parties");
        this->param_names.emplace(u"measurements");
        this->param_names.emplace(u"outcomes");
        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"level");

        this->mutex_params.add_mutex(u"outcomes", u"operators");

        this->min_inputs = 0;
        this->max_inputs = 4;
    }

    MakeMomentMatrixParams::MakeMomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {
        this->output_sequences = this->flags.contains(u"sequences");

        // Either set named params OR give multiple params
        bool set_any_param = this->params.contains(u"parties")
                             || this->params.contains(u"measurements")
                             || this->params.contains(u"operators")
                             || this->params.contains(u"level");

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            // Read and check level - required.
            auto& depth_param = this->find_or_throw(u"level");
            this->hierarchy_level = read_positive_integer(matlabEngine, "Parameter 'level'", depth_param, 0);

            // Read and check number of parties, or default to 1
            auto party_param = this->params.find(u"parties");
            if (party_param != this->params.end()) {
                bool has_opers = this->params.contains(u"operators");
                bool has_mmts = this->params.contains(u"measurements");
                if (!(has_opers || has_mmts)) {
                    throw errors::BadInput{errors::missing_param,
                               "If 'parties' is set, then one of 'operators' or 'measurements' must also be set."};
                }
                this->number_of_parties = read_positive_integer(matlabEngine, "Parameter 'parties'",
                                                                party_param->second, 1);
            } else {
                this->number_of_parties = 1;
            }

            // Read and check measurements
            auto mmt_param = this->params.find(u"measurements");
            if (mmt_param != this->params.end()) {
                this->specification_mode = SpecificationMode::FlatWithMeasurements;
                this->flat_mmts_per_party = read_positive_integer(matlabEngine, "Parameter 'measurements'",
                                                                  mmt_param->second, 1);
            } else {
                this->specification_mode = SpecificationMode::FlatNoMeasurements;
                this->flat_mmts_per_party = 0;
            }

            // Number of operators must also always be specified
            if (this->specification_mode == SpecificationMode::FlatWithMeasurements) {
                auto outcome_param = this->params.find(u"outcomes");
                if (outcome_param == this->params.end()) {
                    throw errors::BadInput{errors::missing_param,
                                           "Parameter 'outcomes' must be set, if 'measurements' is also set."};
                }
                this->flat_outcomes_per_mmt = read_positive_integer(matlabEngine, "Parameter 'outcomes'",
                                                                    outcome_param->second, 1);
            } else if (this->specification_mode == SpecificationMode::FlatNoMeasurements) {
                auto oper_param = this->params.find(u"operators");
                if (oper_param == this->params.end()) {
                    throw errors::BadInput{errors::missing_param,
                                           "Parameter 'operators' must be set, if 'measurements' is not set."};
                }
                if (!castable_to_scalar_int(oper_param->second)) {
                    throw errors::BadInput{errors::missing_param,
                                           "Parameter 'operators' must be a positive scalar integer."};
                }
                this->flat_operators_per_party = read_positive_integer(matlabEngine, "Parameter 'operators'",
                                                                       oper_param->second, 1);
            }

            // All okay!
            return;
        }

        // No named parameters... try to interpret inputs
        if (inputs.size() < 2) {
            std::string errStr{"Please supply either named inputs; or a list of integers in the"};
            errStr += " form of [operators, level], ";
            errStr += "[parties, operators per party, level], ";
            errStr += "or [parties, measurements per party, outcomes per measurement].";
            throw errors::BadInput{errors::too_few_inputs, errStr};
        }

        // Read depth
        this->hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level",
                                                      this->inputs[this->inputs.size()-1], 0);

        // Work out where the operator count should be
        size_t operator_index = 0;
        if (inputs.size() == 2) {
            // Operator_index stays as 0: op, depth
            this->specification_mode = SpecificationMode::FlatNoMeasurements;
        } else if (inputs.size() == 3) {
            operator_index = 1; // party, op, depth
            this->specification_mode = SpecificationMode::FlatNoMeasurements;
        } else if (inputs.size() == 4) {
            operator_index = 2; // party, mmts, ops, depth
            this->specification_mode = SpecificationMode::FlatWithMeasurements;
        }

        // Read measurements (if any) and operator count
        if (this->specification_mode == SpecificationMode::FlatWithMeasurements) {
            this->flat_mmts_per_party =  read_positive_integer(matlabEngine, "Measurement count",
                                                               this->inputs[1], 1);
            this->flat_outcomes_per_mmt = read_positive_integer(matlabEngine, "Number of outcomes",
                                                               this->inputs[operator_index], 1);
        } else {
            this->flat_mmts_per_party = 0;
            this->flat_operators_per_party = read_positive_integer(matlabEngine, "Number of operators",
                                                                this->inputs[operator_index], 1);
        }
    }


    std::unique_ptr<SortedInputs>
    MakeMomentMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        return std::make_unique<MakeMomentMatrixParams>(this->matlabEngine, std::move(input));
    }

    void MakeMomentMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<MakeMomentMatrixParams&>(*inputPtr);

        throw_error(this->matlabEngine, "not_implemented", u"Not implemented");
    }




}