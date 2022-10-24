/**
 * new_algebraic_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "new_algebraic_matrix_system.h"

#include "operators/algebraic/algebraic_context.h"
#include "operators/algebraic/algebraic_matrix_system.h"

#include "storage_manager.h"

#include "utilities/reporting.h"

namespace NPATK::mex::functions {

    namespace {

        std::unique_ptr<AlgebraicContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                       NewAlgebraicMatrixSystemParams& input) {
            std::vector<MonomialSubstitutionRule> rules;
            ShortlexHasher hasher{input.total_operators};
            rules.reserve(input.rules.size());
            for (auto& ir : input.rules) {
                rules.emplace_back(HashedSequence{std::move(ir.LHS), hasher},
                                   HashedSequence{std::move(ir.RHS), hasher}, false);
            }

            return std::make_unique<AlgebraicContext>(input.total_operators, true, std::move(rules));

        }
    }

    NewAlgebraicMatrixSystemParams::NewAlgebraicMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine,
                                                                   SortedInputs &&rawInput)
       : SortedInputs(std::move(rawInput)) {

        // Either set named params OR give multiple params
        bool set_any_param  = this->params.contains(u"operators")
                              || this->params.contains(u"rules");

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }
            this->getFromParams(matlabEngine);
        } else {
            // No named parameters... try to interpret inputs as Settings object + depth
            // Otherwise, try to interpret inputs as flat specification
            this->getFromInputs(matlabEngine);
        }
    }

    void NewAlgebraicMatrixSystemParams::getFromInputs(matlab::engine::MATLABEngine &matlabEngine) {
        if (inputs.empty()) {
            std::string errStr{"Please supply either named parameters; "};
            errStr += " \"number of operators\",";
            errStr += " or \"number of operators, cell array of rules\".";
            throw errors::BadInput{errors::too_few_inputs, errStr};
        }

        this->readOperatorSpecification(matlabEngine, inputs[0], "Number of operators");

        if (inputs.size() > 1) {
            this->rules = read_monomial_rules(matlabEngine, inputs[1], "Rules", this->total_operators);
            // Check length
            check_rule_length(matlabEngine, ShortlexHasher{this->total_operators}, this->rules);
        }
    }

    void NewAlgebraicMatrixSystemParams::getFromParams(matlab::engine::MATLABEngine &matlabEngine) {
        // Read number of operators
        auto oper_param = params.find(u"operators");
        if (oper_param == params.end()) {
            throw errors::BadInput{errors::too_few_inputs, "Missing \"operators\" parameter."};
        }
        this->readOperatorSpecification(matlabEngine, oper_param->second, "Parameter 'operators'");

        auto rules_param = params.find(u"rules");
        if (rules_param == params.end()) {
            return;
        }
        this->rules = read_monomial_rules(matlabEngine, rules_param->second,
                                          "Parameter 'rules'", this->total_operators);
    }

    void NewAlgebraicMatrixSystemParams::readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                                  matlab::data::Array &input,
                                                                  const std::string& paramName) {
        this->total_operators = read_positive_integer(matlabEngine, paramName, input, 1);
    }

    NewAlgebraicMatrixSystem::NewAlgebraicMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                       StorageManager &storage)
           : MexFunction(matlabEngine, storage,
                         MEXEntryPointID::NewAlgebraicMatrixSystem,
                         u"new_algebraic_matrix_system") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"rules");

        this->min_inputs = 0;
        this->max_inputs = 2;
    }

    void NewAlgebraicMatrixSystem::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<NewAlgebraicMatrixSystemParams&>(*inputPtr);

        // Input to context:
        std::unique_ptr<AlgebraicContext> contextPtr{make_context(this->matlabEngine, input)};
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
        std::unique_ptr<MatrixSystem> matrixSystemPtr = std::make_unique<AlgebraicMatrixSystem>(std::move(contextPtr));

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }

    std::unique_ptr<SortedInputs> NewAlgebraicMatrixSystem::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return std::make_unique<NewAlgebraicMatrixSystemParams>(this->matlabEngine, std::move(*input));
    }

}