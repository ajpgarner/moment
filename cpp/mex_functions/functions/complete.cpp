/**
 * complete.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "complete.h"

#include "operators/algebraic/rule_book.h"
#include "operators/algebraic/ostream_rule_logger.h"

#include "fragments/export_monomial_rules.h"
#include "utilities/reporting.h"

#include <sstream>

namespace NPATK::mex::functions {

    namespace {

        RuleBook make_rulebook(matlab::engine::MATLABEngine &matlabEngine,
                                                        ShortlexHasher& hasher,
                                                        CompleteParams& input) {
            std::vector<MonomialSubstitutionRule> rules;
            const size_t max_strlen = hasher.longest_hashable_string();
            rules.reserve(input.rules.size());
            for (auto& ir : input.rules) {
                rules.emplace_back(HashedSequence{std::move(ir.LHS), hasher},
                                   HashedSequence{std::move(ir.RHS), hasher}, false);
            }

            return RuleBook{hasher, rules, input.hermitian_operators};
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
        auto limit_iter = this->params.find(u"limit");
        if (limit_iter != this->params.cend()) {
            this->max_attempts = read_positive_integer(matlabEngine, "Parameter 'limit'", limit_iter->second, 1);
        } else {
            this->max_attempts = 128;
        }

        // Default to Hermitian, but allow non-hermitian overrides
        this->hermitian_operators = !(this->flags.contains(u"nonhermitian"));

        // Try to read raw rules
        this->rules = read_monomial_rules(matlabEngine, inputs[0], "Rules", this->max_operators);

        // If no max operator ID specified, guess by taking the highest value from provided rules
        if (this->max_operators == 0) {
            // Always at least one operator...
            this->max_operators = 1;
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
        }

        // Assert that rule lengths are okay
        check_rule_length(matlabEngine, ShortlexHasher{this->max_operators}, this->rules);

    }

    Complete::Complete(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : MexFunction(matlabEngine, storage,
                      MEXEntryPointID::Complete, u"complete") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"operators");
        this->param_names.emplace(u"limit");

        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"nonhermitian");
        this->mutex_params.add_mutex(u"hermitian", u"nonhermitian");

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void Complete::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<CompleteParams&>(*inputPtr);

        // Output context in verbose mode
        std::stringstream ss;
        std::unique_ptr<OStreamRuleLogger> logger;
        if (this->verbose) {
            logger = std::make_unique<OStreamRuleLogger>(ss);
        }

        // Set up hasher & rules
        ShortlexHasher hasher{input.max_operators};
        RuleBook rules = make_rulebook(this->matlabEngine, hasher, input);

        // Attempt completion
        bool completed = rules.complete(input.max_attempts, logger.get());

        // Print completion log (in verbose mode)
        if (this->verbose) {
            print_to_console(this->matlabEngine, ss.str());
        }

        // Print a warning, if not complete (and not in quiet mode)
        if (!this->quiet && !this->verbose && !completed) {
            print_to_console(this->matlabEngine,
                             "Maximum number of new rules were introduced, but the set was not completed.\n");
        }

        // Output list of parsed rules
        output[0] = export_monomial_rules(rules);
    }

    std::unique_ptr<SortedInputs> Complete::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return std::make_unique<CompleteParams>(this->matlabEngine, std::move(*input));
    }

}