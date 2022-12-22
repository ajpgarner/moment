/**
 * complete.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "complete.h"

#include "scenarios/algebraic/rule_book.h"
#include "scenarios/algebraic/ostream_rule_logger.h"

#include "fragments/export_monomial_rules.h"
#include "utilities/reporting.h"

#include <sstream>

namespace Moment::mex::functions {

    namespace {

        Algebraic::RuleBook make_rulebook(matlab::engine::MATLABEngine &matlabEngine,
                                          ShortlexHasher& hasher, CompleteParams& input) {
            std::vector<Algebraic::MonomialSubstitutionRule> rules;
            const size_t max_strlen = hasher.longest_hashable_string();

            if (input.commutative) {
                rules = Algebraic::RuleBook::commutator_rules(hasher, input.max_operators);
            }

            rules.reserve(rules.size() + input.rules.size());
            for (auto &ir: input.rules) {
                rules.emplace_back(HashedSequence{std::move(ir.LHS), hasher},
                                   HashedSequence{std::move(ir.RHS), hasher}, ir.negated);
            }
            return Algebraic::RuleBook{hasher, rules, input.hermitian_operators};
        }
    }


    CompleteParams::CompleteParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {
        // Do we specify number of operators?
        auto op_iter = this->params.find(u"operators");
        if (op_iter != this->params.cend()) {
            this->max_operators = read_positive_integer(matlabEngine, "Parameter 'operators'", op_iter->second, 0);
        } else {
            this->max_operators = 0;
        }

        // Do we specify number of attempts?
        if (this->flags.contains(u"test")) {
            this->max_attempts = 0;
            this->test_only = true;
        } else {
            auto limit_iter = this->params.find(u"limit");
            if (limit_iter != this->params.cend()) {
                this->max_attempts = read_positive_integer(matlabEngine, "Parameter 'limit'", limit_iter->second, 0);
                this->test_only = (0 == this->max_attempts);
            } else {
                this->max_attempts = 128;
                this->test_only = false;
            }
        }

        // Default to Hermitian, but allow non-hermitian override
        this->hermitian_operators = !(this->flags.contains(u"nonhermitian"));

        // Default to non-commutative, but allow commutative override
        this->commutative = this->flags.contains(u"commutative");

        // Try to read raw rules (w/ matlab indices)
        this->rules = read_monomial_rules(matlabEngine, inputs[0], "Rules", true, this->max_operators);

        // If no max operator ID specified, guess by taking the highest value from provided rules
        if (this->max_operators == 0) {
            // Always at least one operator...
            // Look through rules
            for (const auto& raw_rule : this->rules) {
                // LHS
                for (auto l : raw_rule.LHS) {
                    if (l > this->max_operators) {
                        this->max_operators = l;
                    }
                }

                // RHS
                for (auto r : raw_rule.RHS) {
                    if (r > max_operators) {
                        this->max_operators = r;
                    }
                }
            }
            ++this->max_operators;
        }

        // Assert that rule lengths are okay
        check_rule_length(matlabEngine, ShortlexHasher{this->max_operators}, this->rules);

    }

    Complete::Complete(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : MexFunction(matlabEngine, storage,
                      MEXEntryPointID::Complete, u"complete") {
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"limit");

        this->flag_names.emplace(u"test");

        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"nonhermitian");
        this->mutex_params.add_mutex(u"hermitian", u"nonhermitian");

        this->flag_names.emplace(u"commutative");
        this->flag_names.emplace(u"noncommutative");
        this->mutex_params.add_mutex(u"commutative", u"noncommutative");

        this->mutex_params.add_mutex(u"test", u"limit");

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void Complete::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<CompleteParams&>(*inputPtr);

        // Output context in verbose mode
        std::stringstream ss;
        std::unique_ptr<Algebraic::OStreamRuleLogger> logger;
        if (this->verbose) {
            logger = std::make_unique<Algebraic::OStreamRuleLogger>(ss);
        }

        // Set up hasher & rules
        ShortlexHasher hasher{input.max_operators};
        auto rules = make_rulebook(this->matlabEngine, hasher, input);

        // Print input
        if (this->debug) {
            std::stringstream dss;
            dss << "Input rules:\n" << rules;
            print_to_console(this->matlabEngine, dss.str());
        }

        // Attempt completion
        bool completed = rules.complete(input.max_attempts, logger.get());

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
            print_to_console(this->matlabEngine,
                             "Maximum number of new rules were introduced, but the set was not completed.\n");
        }


        if (input.test_only) {
            // Output completion test result (true/false)
            matlab::data::ArrayFactory factory;
            output[0] = factory.createArray<bool>({1,1}, {completed});

        } else {
            // Output list of parsed rules, using matlab indices
            output[0] = export_monomial_rules(rules, true);

            // Output whether complete or not
            if (output.size()>=2) {
                matlab::data::ArrayFactory factory;
                output[1] = factory.createArray<bool>({1,1}, {completed});
            }
        }

    }

    std::unique_ptr<SortedInputs> Complete::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return std::make_unique<CompleteParams>(this->matlabEngine, std::move(*input));
    }

}