/**
 * complete.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "complete.h"

#include "errors.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/name_table.h"
#include "scenarios/algebraic/operator_rulebook.h"
#include "scenarios/algebraic/ostream_rule_logger.h"

#include "import/read_operator_names.h"
#include "export/export_operator_rules.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include <sstream>

namespace Moment::mex::functions {

    namespace {

        Algebraic::OperatorRulebook make_rulebook(matlab::engine::MATLABEngine &matlabEngine,
                                                  CompleteParams& input) {
            using namespace Algebraic;

            std::vector<Algebraic::OperatorRule> rules;
            const auto& apc = *input.apc;

            if (input.commutative) {
                Algebraic::OperatorRulebook::commutator_rules(apc, rules);
            }
            if (!apc.self_adjoint() && input.normal_operators) {
                Algebraic::OperatorRulebook::normal_rules(apc, rules);
            }

            rules.reserve(rules.size() + input.rules.size());
            size_t rule_index = 0;
            for (auto &ir: input.rules) {
                rules.emplace_back(ir.to_rule(matlabEngine, apc, rule_index));
                ++rule_index;
            }
            return Algebraic::OperatorRulebook{apc, rules};
        }
    }

    CompleteParams::CompleteParams(SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {

        // Default to Hermitian, but allow non-hermitian override
        auto hermitian_mode = [&]() {
            if (this->flags.contains(u"nonhermitian") || this->flags.contains(u"bunched")) {
                return Algebraic::AlgebraicPrecontext::ConjugateMode::Bunched;
            } else if (this->flags.contains(u"interleaved")) {
               return Algebraic::AlgebraicPrecontext::ConjugateMode::Interleaved;
            }
            return Algebraic::AlgebraicPrecontext::ConjugateMode::SelfAdjoint;
        }();

        if (hermitian_mode != Algebraic::AlgebraicPrecontext::ConjugateMode::SelfAdjoint) {
            this->normal_operators = this->flags.contains(u"normal");
        } else {
            this->normal_operators = true;
        }

        // Do we specify number of operators, or list of names
        if ((inputs[0].getType() == matlab::data::ArrayType::CHAR)
            || (inputs[0].getType() == matlab::data::ArrayType::MATLAB_STRING)) {

            this->max_operators = get_name_table_length(matlabEngine, "Operator specification", inputs[0]);
            this->apc = std::make_unique<Algebraic::AlgebraicPrecontext>(this->max_operators, hermitian_mode);
            this->names = read_name_table(matlabEngine, *this->apc, "Operator specification",  inputs[0]);
            assert(this->names);
        } else {
            this->max_operators = read_positive_integer<size_t>(matlabEngine, "Operator specification", inputs[0], 1);
            this->apc = std::make_unique<Algebraic::AlgebraicPrecontext>(this->max_operators, hermitian_mode);
            this->names = std::make_unique<Algebraic::NameTable>(*apc);
        }

        // Do we specify number of attempts?
        if (this->flags.contains(u"test")) {
            this->max_attempts = 0;
            this->test_only = true;
        } else {
            auto limit_iter = this->params.find(u"limit");
            if (limit_iter != this->params.cend()) {
                this->max_attempts = read_positive_integer<size_t>(matlabEngine, "Parameter 'limit'",
                                                                   limit_iter->second, 0);
                this->test_only = (0 == this->max_attempts);
            } else {
                this->max_attempts = 128;
                this->test_only = false;
            }
        }

        // Default to non-commutative, but allow commutative override
        this->commutative = this->flags.contains(u"commutative");

        // Check we have positive number of operators in our system
        if (this->max_operators == 0) {
            throw BadParameter{"Cannot automatically infer operator count."};
        }
        assert(this->apc);

        this->rules = read_monomial_rules(matlabEngine, inputs[1], "MonomialRules", true, *this->apc, *this->names);

        // Assert that rule lengths are okay
        check_rule_length(matlabEngine, apc->hasher, this->rules);
    }

    CompleteParams::~CompleteParams() noexcept = default;


    Complete::Complete(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->param_names.emplace(u"limit");

        this->flag_names.emplace(u"test");

        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"nonhermitian");
        this->flag_names.emplace(u"bunched");
        this->flag_names.emplace(u"interleaved");
        this->mutex_params.add_mutex({u"hermitian", u"nonhermitian", u"bunched", u"interleaved"});

        this->flag_names.emplace(u"normal");

        this->flag_names.emplace(u"commutative");
        this->flag_names.emplace(u"noncommutative");
        this->mutex_params.add_mutex(u"commutative", u"noncommutative");

        this->mutex_params.add_mutex(u"test", u"limit");

        this->min_inputs = 2;
        this->max_inputs = 2;
    }

    void Complete::operator()(IOArgumentRange output, CompleteParams &input) {
        // Output context in verbose mode
        std::stringstream ss;
        std::unique_ptr<Algebraic::OStreamRuleLogger> logger;
        if (this->verbose) {
            logger = std::make_unique<Algebraic::OStreamRuleLogger>(ss, input.names.get());
        }

        // Set up rules
        auto rules = make_rulebook(this->matlabEngine, input);

        // Print input
        if (this->debug) {
            std::stringstream dss;
            dss << "Input rules:\n" << rules;
            print_to_console(this->matlabEngine, dss.str());
        }

        // Attempt completion
        bool completed = [&]() {
            try {
                return rules.complete(input.max_attempts, logger.get());
            } catch (std::exception& e) {
                throw BadParameter{e.what()};
            }
        }();


        // Print completion log (in verbose mode)
        if (this->verbose) {
            if (this->debug) {
                ss << "Max operators: " << input.max_operators << "\n";
                ss << "Output rules:\n" << rules;
            }
            print_to_console(this->matlabEngine, ss.str());
        }

        // Print a warning, if not complete (and not in quiet mode, or a test)
        if (!completed && !input.test_only && !this->quiet && !this->verbose) {
            print_warning(this->matlabEngine,
                          "Maximum number of new rules were introduced, but the set was not completed.\n");
        }

        if (input.test_only) {
            // Output completion test result (true/false)
            matlab::data::ArrayFactory factory;
            output[0] = factory.createArray<bool>({1,1}, {completed});

        } else {
            // Output list of parsed rules, using matlab indices
            matlab::data::ArrayFactory factory;
            OperatorRuleExporter exporter{this->matlabEngine, factory, true};
            output[0] = exporter(rules);

            // Output whether complete or not
            if (output.size()>=2) {
                output[1] = factory.createArray<bool>({1,1}, {completed});
            }
        }

    }


}