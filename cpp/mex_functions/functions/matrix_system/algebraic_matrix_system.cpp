/**
 * algebraic_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_matrix_system.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"
#include "scenarios/algebraic/ostream_rule_logger.h"

#include "storage_manager.h"

#include "import/read_operator_names.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {
    namespace {
        std::unique_ptr<Algebraic::AlgebraicContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                                  AlgebraicMatrixSystemParams& input) {
            std::vector<Algebraic::OperatorRule> rules;
            const auto& apc = *input.apc;

            rules.reserve(input.rules.size());
            size_t rule_index = 0;
            for (auto &ir: input.rules) {
                rules.emplace_back(ir.to_rule(matlabEngine, apc, rule_index));
                ++rule_index;
            }

            return std::make_unique<Algebraic::AlgebraicContext>(*input.apc, std::move(input.names),
                                                                 input.commutative, input.normal_operators,
                                                                 rules);
        }

        // Default to Hermitian, but allow non-hermitian override
        auto get_hermitian_mode(const AlgebraicMatrixSystemParams& params) {
            if (params.flags.contains(u"nonhermitian") || params.flags.contains(u"bunched")) {
                return Algebraic::AlgebraicPrecontext::ConjugateMode::Bunched;
            } else if (params.flags.contains(u"interleaved")) {
                return Algebraic::AlgebraicPrecontext::ConjugateMode::Interleaved;
            }
            return Algebraic::AlgebraicPrecontext::ConjugateMode::SelfAdjoint;
        };
    }

    AlgebraicMatrixSystemParams::AlgebraicMatrixSystemParams(SortedInputs &&rawInput)
       : SortedInputs(std::move(rawInput)) {

        // Any completion requested?
        auto complete_param = params.find(u"complete_attempts");
        if (complete_param != params.end()) {
            this->complete_attempts = read_positive_integer<size_t>(matlabEngine, "Parameter 'complete_attempts'",
                                                                    complete_param->second, 0);
        } else {
            this->complete_attempts = 0;
        }

        if (get_hermitian_mode(*this) != Algebraic::AlgebraicPrecontext::ConjugateMode::SelfAdjoint) {
            this->normal_operators = this->flags.contains(u"normal");
        } else {
            this->normal_operators = true;
        }

        // Is tolerance set?
        auto tolerance_param_iter = this->params.find(u"tolerance");
        if (tolerance_param_iter != this->params.cend()) {
            this->zero_tolerance = read_as_double(this->matlabEngine, tolerance_param_iter->second);
            if (this->zero_tolerance < 0) {
                throw BadParameter{"Tolerance must be non-negative value."};
            }
        }


        // Default to non-commutative, but allow commutative override
        this->commutative = this->flags.contains(u"commutative");

        // Either set named params OR give multiple params
        bool set_any_param  = this->params.contains(u"operators")
                              || this->params.contains(u"rules");


        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw BadParameter{"Input arguments should be exclusively named, or exclusively unnamed."};
            }
            this->getFromParams(matlabEngine);
        } else {
            // No named parameters... try to interpret inputs as Settings object + depth
            // Otherwise, try to interpret inputs as flat specification
            this->getFromInputs(matlabEngine);
        }
    }

    void AlgebraicMatrixSystemParams::getFromInputs(matlab::engine::MATLABEngine &matlabEngine) {
        if (inputs.empty()) {
            std::string errStr{"Please supply either named parameters; "};
            errStr += " \"number of operators\",";
            errStr += " or \"number of operators, cell array of rules\".";
            throw InputCountException{"algebraic_matrix_system", 1, 3, 0, errStr};
        }

        this->readOperatorSpecification(matlabEngine, inputs[0], "Number of operators");

        if (inputs.size() > 1) {
            assert(this->names);
            assert(this->apc);

            this->rules = read_monomial_rules(matlabEngine, inputs[1], "MonomialRules", true, *this->apc, *this->names);
            check_rule_length(matlabEngine, apc->hasher, this->rules);
        }
    }

    void AlgebraicMatrixSystemParams::getFromParams(matlab::engine::MATLABEngine &matlabEngine) {
        // Read number of operators
        this->find_and_parse_or_throw(u"operators", [this, &matlabEngine](matlab::data::Array& operator_param) {
            this->readOperatorSpecification(matlabEngine, operator_param, "Parameter 'operators'");
        });

        // Read supplied rules
        this->find_and_parse(u"rules", [this, &matlabEngine](matlab::data::Array& rules_param) {
            assert(this->names);
            assert(this->apc);

            this->rules = read_monomial_rules(matlabEngine, rules_param,
                                              "Parameter 'rules'", true, *this->apc, *this->names);
            check_rule_length(matlabEngine, apc->hasher, this->rules);
        });
    }

    void AlgebraicMatrixSystemParams::readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                                matlab::data::Array &input,
                                                                const std::string& paramName) {
        // Is operator argument a single string?
        if ((input.getType() == matlab::data::ArrayType::CHAR)
            || (input.getType() == matlab::data::ArrayType::MATLAB_STRING)) {

            this->total_operators = get_name_table_length(matlabEngine, paramName, input);
            this->apc = std::make_unique<Algebraic::AlgebraicPrecontext>(
                    this->total_operators,
                    get_hermitian_mode(*this)
            );
            this->names = read_name_table(matlabEngine, *this->apc, paramName, input);
            assert(names);
            return;
        }

        // Otherwise, assume operator argument is a number, and auto-generate names
        this->total_operators = read_positive_integer<size_t>(matlabEngine, paramName, input, 1);
        this->apc = std::make_unique<Algebraic::AlgebraicPrecontext>(
                this->total_operators,
                get_hermitian_mode(*this)
        );
        this->names = std::make_unique<Algebraic::NameTable>(*this->apc);
    }

    AlgebraicMatrixSystemParams::~AlgebraicMatrixSystemParams() = default;

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                 StorageManager &storage)
           : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"rules");
        this->param_names.emplace(u"complete_attempts");

        this->param_names.emplace(u"tolerance");

        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"nonhermitian");
        this->flag_names.emplace(u"bunched");
        this->flag_names.emplace(u"interleaved");
        this->mutex_params.add_mutex({u"hermitian", u"nonhermitian", u"bunched", u"interleaved"});

        this->flag_names.emplace(u"normal");

        this->flag_names.emplace(u"commutative");
        this->flag_names.emplace(u"noncommutative");
        this->mutex_params.add_mutex(u"commutative", u"noncommutative");


        this->min_inputs = 0;
        this->max_inputs = 2;
    }

    void AlgebraicMatrixSystem::operator()(IOArgumentRange output, AlgebraicMatrixSystemParams &input) {

        // Input to context:
        std::unique_ptr<Algebraic::AlgebraicContext> contextPtr{make_context(this->matlabEngine, input)};
        if (!contextPtr) {
            throw InternalError{"Context object could not be created."};
        }

        // Try to complete context, if requested
        bool complete_rules;
        if (input.complete_attempts > 0) {
            std::stringstream ss;
            std::unique_ptr<Algebraic::OStreamRuleLogger> logger;
            if (this->verbose) {
                ss << "Attempting completion of ruleset:\n";
                logger = std::make_unique<Algebraic::OStreamRuleLogger>(ss, &(contextPtr->names()));
            }
            complete_rules = contextPtr->attempt_completion(input.complete_attempts, logger.get());
            if (this->verbose) {
                ss << "\n";
                print_to_console(this->matlabEngine, ss.str());
            }
        } else {
            // Otherwise, just test for completeness
            complete_rules = contextPtr->is_complete();
        }

        // Output context in verbose mode
        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Give warning if rules are incomplete
        if (!this->quiet && !complete_rules) {
            std::stringstream ss;
            ss << "Supplied ruleset was not completed.\n"
               << "This may result in missed algebraic substitutions and unpredictable behaviour,\n"
               << "especially for lower-order operator matrices.\n";
            print_warning(this->matlabEngine, ss.str());
        }

        // Make new system around context
        std::unique_ptr<MatrixSystem> matrixSystemPtr =
                std::make_unique<Algebraic::AlgebraicMatrixSystem>(std::move(contextPtr), input.zero_tolerance);

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }


}