/**
 * new_algebraic_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "new_algebraic_matrix_system.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"
#include "scenarios/algebraic/ostream_rule_logger.h"

#include "storage_manager.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    namespace {

        std::unique_ptr<Algebraic::AlgebraicContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                       NewAlgebraicMatrixSystemParams& input) {

            // First, create name table (or error trying)
            std::unique_ptr<Algebraic::NameTable> name_table;
            if (input.names.empty()) {
                name_table = std::make_unique<Algebraic::NameTable>(input.total_operators);
            } else {
                try {
                    name_table = std::make_unique<Algebraic::NameTable>(std::move(input.names));
                }
                catch (const std::invalid_argument& iae) {
                    throw_error(matlabEngine, errors::bad_param, iae.what());
                }
            }

            std::vector<Algebraic::MonomialSubstitutionRule> rules;
            Algebraic::AlgebraicPrecontext apc{static_cast<oper_name_t>(input.total_operators),
                                               input.hermitian_operators};

            const auto max_strlen = apc.hasher.longest_hashable_string();

            rules.reserve(input.rules.size());
            size_t rule_index = 0;
            for (auto& ir : input.rules) {
                if (ir.LHS.size() > max_strlen) {
                    std::stringstream errSS;
                    errSS << "Error with rule #" + std::to_string(rule_index+1) + ": LHS too long.";
                    throw_error(matlabEngine, errors::bad_param, errSS.str());
                }
                if (ir.RHS.size() > max_strlen) {
                    std::stringstream errSS;
                    errSS << "Error with rule #" + std::to_string(rule_index+1) + ": RHS too long.";
                    throw_error(matlabEngine, errors::bad_param, errSS.str());
                }
                try {
                    rules.emplace_back(HashedSequence{sequence_storage_t(ir.LHS.begin(), ir.LHS.end()), apc.hasher},
                                       HashedSequence{sequence_storage_t(ir.RHS.begin(), ir.RHS.end()), apc.hasher},
                                       ir.negated);
                } catch (Moment::Algebraic::errors::invalid_rule& ire) {
                    std::stringstream errSS;
                    errSS << "Error with rule #" + std::to_string(rule_index+1) + ": " << ire.what();
                    throw_error(matlabEngine, errors::bad_param, errSS.str());
                }
                ++rule_index;
            }

            return std::make_unique<Algebraic::AlgebraicContext>(std::move(name_table),
                                                                 input.hermitian_operators,
                                                                 input.commutative, input.normal_operators,
                                                                 rules);
        }
    }

    NewAlgebraicMatrixSystemParams::NewAlgebraicMatrixSystemParams(SortedInputs &&rawInput)
       : SortedInputs(std::move(rawInput)) {

        // Any completion requested?
        auto complete_param = params.find(u"complete_attempts");
        if (complete_param != params.end()) {
            this->complete_attempts = read_positive_integer<size_t>(matlabEngine, "Parameter 'complete_attempts'",
                                                                    complete_param->second, 0);
        } else {
            this->complete_attempts = 0;
        }

        // Default to Hermitian, but allow non-hermitian override
        this->hermitian_operators = !(this->flags.contains(u"nonhermitian"));
        if (!this->hermitian_operators) {
            this->normal_operators = this->flags.contains(u"normal");
        } else {
            this->normal_operators = true;
        }

        // Default to non-commutative, but allow commutative override
        this->commutative = this->flags.contains(u"commutative");

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
            const auto max_ops = this->total_operators * (this->hermitian_operators ? 1 : 2);
            this->rules = read_monomial_rules(matlabEngine, inputs[1], "Rules", true, max_ops);
            check_rule_length(matlabEngine, ShortlexHasher{max_ops}, this->rules);
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

        const auto max_ops = this->total_operators * (this->hermitian_operators ? 1 : 2);
        this->rules = read_monomial_rules(matlabEngine, rules_param->second,
                                          "Parameter 'rules'", true, max_ops);
        check_rule_length(matlabEngine, ShortlexHasher{max_ops}, this->rules);
    }

    void NewAlgebraicMatrixSystemParams::readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                                                  matlab::data::Array &input,
                                                                  const std::string& paramName) {
        // Is operator argument a single string?
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            auto name_char_array = static_cast<matlab::data::CharArray>(input);
            auto name_str = name_char_array.toAscii();
            this->names.reserve(name_str.size());
            for (auto x : name_str) {
                this->names.emplace_back(1, x);
            }
            this->total_operators = this->names.size();
            return;
        }

        // Is operator argument an array of strings?
        if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            auto mls_array = static_cast<matlab::data::TypedArray<matlab::data::MATLABString>>(input);
            this->names.reserve(mls_array.getNumberOfElements());
            for (auto elem : mls_array) {
                if (elem.has_value()) {
                    this->names.emplace_back(static_cast<std::string>(elem));
                }
            }
            this->total_operators = this->names.size();
            return;
        }

        // Otherwise, assume operator argument is a number.
        this->total_operators = read_positive_integer<size_t>(matlabEngine, paramName, input, 1);
    }

    NewAlgebraicMatrixSystem::NewAlgebraicMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                       StorageManager &storage)
           : ParameterizedMexFunction(matlabEngine, storage, u"new_algebraic_matrix_system") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"rules");
        this->param_names.emplace(u"complete_attempts");

        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"nonhermitian");
        this->mutex_params.add_mutex(u"hermitian", u"nonhermitian");

        this->flag_names.emplace(u"normal");

        this->flag_names.emplace(u"commutative");
        this->flag_names.emplace(u"noncommutative");
        this->mutex_params.add_mutex(u"commutative", u"noncommutative");

        this->min_inputs = 0;
        this->max_inputs = 2;
    }

    void NewAlgebraicMatrixSystem::operator()(IOArgumentRange output, NewAlgebraicMatrixSystemParams &input) {
        using namespace Algebraic;

        // Input to context:
        std::unique_ptr<AlgebraicContext> contextPtr{make_context(this->matlabEngine, input)};
        if (!contextPtr) {
            throw_error(this->matlabEngine, errors::internal_error, "Context object could not be created.");
        }

        // Try to complete context, if requested
        bool complete_rules;
        if (input.complete_attempts > 0) {
            std::stringstream ss;
            std::unique_ptr<OStreamRuleLogger> logger;
            if (this->verbose) {
                ss << "Attempting completion of ruleset:\n";
                logger = std::make_unique<OStreamRuleLogger>(ss);
            }
            complete_rules = contextPtr->attempt_completion(input.complete_attempts, logger.get());
            if (this->verbose) {
                ss << "\n";
                print_to_console(this->matlabEngine, ss.str());
            }
        } else {
            // Otherwise, just test for completeness
            complete_rules = contextPtr->rulebook().is_complete();
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
            ss << "WARNING: Supplied ruleset was not completed.\n"
               << "This may result in missed algebraic substitutions "
               << "and unpredictable behaviour, especially for lower-order operator matrices.\n";
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


}