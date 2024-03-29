/**
 * create_moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "create_moment_rules.h"

#include "storage_manager.h"

#include "scenarios/context.h"

#include "symbolic/monomial_comparator_by_hash.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/symbol_table.h"

#include "import/read_opseq_polynomial.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_string.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/read_choice.h"

namespace Moment::mex::functions {
    CreateMomentRulesParams::CreateMomentRulesParams(SortedInputs &&rawInput)
            : SortedInputs{std::move(rawInput)}, matrix_system_key{matlabEngine} {
        // Read matrix key
        this->matrix_system_key.parse_input(inputs[0]);

        // Ascertain input mode
        if (this->flags.contains(u"info")) {
            this->input_mode = InputMode::InformationOnly;
        } else {
            auto input_mode_iter = this->params.find(u"input");
            if (input_mode_iter != this->params.cend()) {
                try {
                    switch (read_choice("Parameter 'input'",
                                        {"list", "symbols", "sequences"},
                                        input_mode_iter->second)) {
                        case 0:
                            this->input_mode = InputMode::SubstitutionList;
                            break;
                        case 1:
                            this->input_mode = InputMode::FromSymbolIds;
                            break;
                        case 2:
                            this->input_mode = InputMode::FromOperatorSequences;
                            break;
                    }
                } catch (const Moment::mex::errors::invalid_choice &ice) {
                    throw BadParameter{ice.what()};
                }
            }
        }

        // Special case: info only mode
        if (this->info_only_mode()) {
            // Check we don't set any other params in info only mode
            if (this->params.contains(u"label")
                || this->params.contains(u"order")
                || this->params.contains(u"tolerance")
                || this->flags.contains(u"no_factors")
                || this->flags.contains(u"no_new_symbols")) {
                throw BadParameter{"No additional creation parameters can be set when in 'info' mode."};
            }
            if (this->params.contains(u"rulebook")) {
                throw BadParameter{
                    "In 'info' mode, rulebook should be provided as the function argument, not as a named parameter."
                };
            }

            // Attempt to read last param as rulebook key.
            this->existing_rule_key = read_positive_integer<size_t>(matlabEngine, "Rulebook index", inputs[1], 0);
            return;
        }

        // Do we have a label?
        auto label_param_iter = this->params.find(u"label");
        if (label_param_iter != this->params.cend()) {
            auto maybe_str = read_as_utf8(label_param_iter->second);
            if (maybe_str.has_value()) {
                this->human_readable_name = std::move(maybe_str.value());
            } else {
                throw BadParameter{"If 'label' is set, it cannot be empty."};
            }
        }

        // Merge into existing rule-set?
        auto rulebook_param_iter = this->params.find(u"rulebook");
        if (rulebook_param_iter != this->params.cend()) {
            this->existing_rule_key = read_as_uint64(this->matlabEngine, rulebook_param_iter->second);
            this->merge_into_existing = true;
        } else {
            this->merge_into_existing = false;
        }

        // Do we automatically add rules arising from factorization?
        if (this->flags.contains(u"no_factors")) {
            this->infer_from_factors = false;
        }

        // Do we automatically register new symbols, if they are specified?
        if (this->flags.contains(u"no_new_symbols")) {
            this->create_missing_symbols = false;
        }

        // Extra import
        switch (this->input_mode) {
            case InputMode::SubstitutionList:
                parse_as_sublist(this->inputs[1]);
                break;
            case InputMode::FromSymbolIds:
                parse_as_symbol_polynomials(this->inputs[1]);
                break;
            case InputMode::FromOperatorSequences:
                parse_as_op_seq_polynomials(this->inputs[1]);
                break;
            default:
                throw BadParameter{"Unknown input mode."};
        }
    }

    CreateMomentRulesParams::~CreateMomentRulesParams() noexcept = default;

    void CreateMomentRulesParams::parse_as_sublist(const matlab::data::Array &input) {
        this->sub_list.clear();

        // Empty input can be interpreted as empty substitution list, and so is valid
        if (input.isEmpty()) {
            return;
        }

        // Input must be cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << "Substitution list should be provided as a cell array.";
            throw BadParameter{errSS.str()};
        }

        // Cast input to cell array [matlab says this will be a reference, not copy. We can hope...]
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t sub_count = cell_input.getNumberOfElements();

        // Go through elements
        for (size_t index = 0; index < sub_count; ++index) {
            const auto the_cell = cell_input[index];
            if (the_cell.getType() != matlab::data::ArrayType::CELL) {
                std::stringstream errSS;
                errSS << "Substitution list element " << (index+1) << " must be a cell array.";
                throw BadParameter{errSS.str()};
            }
            const matlab::data::CellArray the_cell_as_cell = the_cell;
            if (the_cell_as_cell.getNumberOfElements() != 2) {
                std::stringstream errSS;
                errSS << "Substitution list element " << (index+1) << " must have two elements: {symbol id, value}.";
                throw BadParameter{errSS.str()};
            }

            auto symbol_id = read_as_scalar<symbol_name_t>(this->matlabEngine, the_cell_as_cell[0]);
            auto value = read_as_complex_scalar<double>(this->matlabEngine, the_cell_as_cell[1]);

            // Cursory validation of symbol_id (must be non-negative, and not 0 or 1)
            if (symbol_id < 2) {
                std::stringstream  errSS;
                errSS << "Substitution list element " << (index+1);
                if (symbol_id < 0) {
                    errSS << " cannot be negative.";
                } else {
                    errSS << " cannot bind reserved symbol \"" << symbol_id << "\".";
                }
                throw BadParameter{errSS.str()};
            }
            this->sub_list.emplace(std::make_pair(symbol_id, value));
        }
    }

    void CreateMomentRulesParams::parse_as_symbol_polynomials(const matlab::data::Array &input) {
        this->raw_symbol_polynomials.clear();

        // Empty input can be interpreted as empty substitution list, and so is valid
        if (input.isEmpty()) {
            return;
        }

        // Input must be cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << "Symbol polynomial list should be provided as a cell array.";
            throw BadParameter{errSS.str()};
        }

        // Cast input to cell array [matlab says this will be a reference, not copy. We can hope...]
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t sub_count = cell_input.getNumberOfElements();
        this->raw_symbol_polynomials.reserve(sub_count);

        // Go through elements
        for (size_t index = 0; index < sub_count; ++index) {
            std::stringstream ruleNameSS;
            ruleNameSS << "Rule #" << (index+1);
            this->raw_symbol_polynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine,
                                                                               ruleNameSS.str(),
                                                                               cell_input[index]));
        }
    }

    void CreateMomentRulesParams::parse_as_op_seq_polynomials(const matlab::data::Array& input) {
        // Empty input can be interpreted as empty substitution list, and so is valid
        if (input.isEmpty()) {
            this->raw_op_seq_polynomials.clear();
            return;
        }

        // Input must be cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << "Operator polynomial list should be provided as a cell array.";
            throw BadParameter{errSS.str()};
        }

        // Cast input to cell array [matlab says this kind of cast produces references, not copies. We can only hope...]
        const auto cell_input = static_cast<const matlab::data::CellArray>(input);
        const size_t rule_count = cell_input.getNumberOfElements();
        this->raw_op_seq_polynomials.reserve(rule_count);

        // Go through elements, and read.
        for (size_t rule_index = 0; rule_index < rule_count; ++rule_index) {
            std::stringstream ruleNameSS;
            ruleNameSS << "Rule #" << (rule_index+1);
            this->raw_op_seq_polynomials.emplace_back(
                    std::make_unique<StagingPolynomial>(this->matlabEngine, cell_input[rule_index], ruleNameSS.str())
            );
        }
    }


    CreateMomentRules::CreateMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;

        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"info");
        this->param_names.emplace(u"input");
        this->mutex_params.add_mutex({u"info", u"input"});

        this->param_names.emplace(u"output");

        this->param_names.emplace(u"label");
        this->param_names.emplace(u"rulebook");

        this->flag_names.emplace(u"no_factors");
        this->flag_names.emplace(u"no_new_symbols");
        this->flag_names.emplace(u"complete_only");
    }

    std::ostream& operator<<(std::ostream& os, CreateMomentRulesParams::InputMode mode) {
        switch (mode) {
            default:
            case CreateMomentRulesParams::InputMode::Unknown:
                os << "[Unknown: " << static_cast<uint64_t>(mode) << "]";
                break;
            case CreateMomentRulesParams::InputMode::InformationOnly:
                os << "InformationOnly";
                break;
            case CreateMomentRulesParams::InputMode::SubstitutionList:
                os << "SubstitutionList";
                break;
            case CreateMomentRulesParams::InputMode::FromSymbolIds:
                os << "FromSymbolIds";
                break;
            case CreateMomentRulesParams::InputMode::FromOperatorSequences:
                os << "FromOperatorSequences";
                break;
        }
        return os;
    }

    void CreateMomentRules::operator()(IOArgumentRange output, CreateMomentRulesParams &input) {
        // Get stored moment matrix
        auto msPtr = input.matrix_system_key(this->storageManager);
        auto& system = *msPtr;

        // Extra info, if in debug mode
        if (this->debug) {
            std::stringstream infoSS;
            infoSS << "Supplied rules as " << input.input_mode << "\n";
            print_to_console(this->matlabEngine, infoSS.str());
        }

        // Create rule-book with new rules
        std::unique_ptr<MomentRulebook> rulebookPtr;
        if (!input.info_only_mode()) {
            rulebookPtr = this->create_rulebook(system, input);
        }

        // Add or merge rulebooks
        auto [rb_id, rulebook] = [&]() -> std::pair<size_t, const MomentRulebook&> {
            // Either, just get information:
            if (input.info_only_mode()) {
                return {input.existing_rule_key, system.Rulebook(input.existing_rule_key)};
            }
            // Or, add/merge for information
            if (input.merge_into_existing) {
                return system.Rulebook.merge_in(input.existing_rule_key, std::move(*rulebookPtr));
            } else {
                // Register rulebook with matrix system
                return system.Rulebook.add(std::move(rulebookPtr));
            }
        }();

        // Prepare to write-out information
        auto new_read_lock = msPtr->get_read_lock();

        // Extra info, if verbose
        if (this->verbose) {
            std::stringstream infoSS;
            infoSS << "Rulebook #" << rb_id << ": " << rulebook.name() << "\n";
            const size_t rb_size = rulebook.size();
            infoSS << "Contains " << rb_size << ((rb_size != 1) ? " rules" : " rule") << ".\n";
            if (rulebook.is_hermitian()) {
                infoSS << "Is hermitian-preserving.\n";
            } else {
                infoSS << "Is not hermitian-preserving.\n";
            }
            if (rulebook.is_monomial()) {
                infoSS << "Is monomial-preserving.\n";
            } else {
                infoSS << "Is not monomial-preserving.\n";
            }
            print_to_console(this->matlabEngine, infoSS.str());
        }

        // Output index
        if (output.size() >= 1) {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar<uint64_t>(rb_id);
        }
    }

    std::unique_ptr<MomentRulebook>
    CreateMomentRules::create_rulebook(MatrixSystem& system, CreateMomentRulesParams& input) const {
        std::unique_ptr<MomentRulebook> book;
        switch (input.input_mode) {
            case CreateMomentRulesParams::InputMode::SubstitutionList:
                book = this->create_rulebook_from_sublist(system, input);
                break;
            case CreateMomentRulesParams::InputMode::FromSymbolIds:
                book = this->create_rulebook_from_symbols(system, input);
                break;
            case CreateMomentRulesParams::InputMode::FromOperatorSequences:
                if (input.create_missing_symbols) {
                    book = this->create_rulebook_from_new_sequences(system, input);
                } else {
                    book = this->create_rulebook_from_existing_sequences( system, input);
                }
                break;
            default:
            case CreateMomentRulesParams::InputMode::Unknown:
                throw InternalError{"Unknown rules input mode."};
        }
        assert(book);

        return book;
    }


    std::unique_ptr<MomentRulebook>
    CreateMomentRules::create_rulebook_from_sublist(MatrixSystem& system, CreateMomentRulesParams& input) const {
        // Lock to read
        auto read_lock = system.get_read_lock();
        auto& symbols = system.Symbols();

        // Range check sublist vs. symbol table
        size_t idx = 0;
        for (auto [id, val] : input.sub_list) {
            if (id >= symbols.size()) {
                std::stringstream errSS;
                errSS << "Symbol " << id << " not found (substitution list element " << (idx+1) << ")";
                throw BadParameter{errSS.str()};
            }
            ++idx;
        }

        if (this->debug) {
            std::stringstream ss;
            ss << "CreateMomentRules::create_rulebook_from_sublist"
               << " parsed " << input.sub_list.size() << " rules.\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Make empty rulebook
        auto output = std::make_unique<MomentRulebook>(system, input.infer_from_factors);
        if (!input.human_readable_name.empty()) {
            output->set_name(input.human_readable_name);
        }

        // Import rules, and compile
        output->add_raw_rules(input.sub_list);
        output->complete();

        return output;
    }

    std::unique_ptr<MomentRulebook>
    CreateMomentRules::create_rulebook_from_symbols(MatrixSystem& system, CreateMomentRulesParams& input) const {
        // Lock to read
        auto read_lock = system.get_read_lock();
        auto& symbols = system.Symbols();

        // Range check data vs. symbol table
        size_t idx = 0;
        for (const auto& rule : input.raw_symbol_polynomials) {
            size_t elem_idx = 0;
            for (const auto& elem : rule) {
                if (elem.symbol_id >= symbols.size()) {
                    std::stringstream errSS;
                    errSS << "Symbol " << elem.symbol_id << " not found "
                          << "(rule #" << (idx+1) << ",  element " << (elem_idx+1) << ").";
                    throw BadParameter{errSS.str()};
                }
                ++elem_idx;
            }
            ++idx;
        }

        // Construct empty ruleset with ordering
        auto output = std::make_unique<MomentRulebook>(system, input.infer_from_factors);
        if (!input.human_readable_name.empty()) {
            output->set_name(input.human_readable_name);
        }

        // Read rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_symbol_polynomials.size());
        for (const auto& raw_rule : input.raw_symbol_polynomials) {
            raw_polynomials.emplace_back(raw_data_to_polynomial(this->matlabEngine, output->factory, raw_rule));
        }

        // Import rules, and compile
        output->add_raw_rules(std::move(raw_polynomials));
        output->complete();

        return output;
    }


    std::unique_ptr<MomentRulebook>
    CreateMomentRules::create_rulebook_from_new_sequences(Moment::MatrixSystem &system,
                                                          CreateMomentRulesParams &input) const {
        auto write_lock = system.get_write_lock();
        auto& symbols = system.Symbols();
        for (auto& rawPoly : input.raw_op_seq_polynomials) {
            rawPoly->supply_context(system.Context());
            rawPoly->find_or_register_symbols(symbols);
        }

        //Make empty rulebook and get factory...
        auto output = std::make_unique<MomentRulebook>(system, input.infer_from_factors);
        if (!input.human_readable_name.empty()) {
            output->set_name(input.human_readable_name);
        }

        // Import rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_op_seq_polynomials.size());
        for (auto& raw_rule : input.raw_op_seq_polynomials) {
            raw_polynomials.emplace_back(raw_rule->to_polynomial(output->factory));
        }
        output->add_raw_rules(std::move(raw_polynomials));

        // Compile and return
        output->complete();
        return output;
    }

    std::unique_ptr<MomentRulebook>
    CreateMomentRules::create_rulebook_from_existing_sequences(MatrixSystem& system,
                                                               CreateMomentRulesParams& input) const {
        auto read_lock = system.get_read_lock();
        auto& symbols = system.Symbols();
        for (auto& rawPoly : input.raw_op_seq_polynomials) {
            rawPoly->supply_context(system.Context());
            rawPoly->find_symbols(symbols);
        }

        //Make empty rulebook and get factory...
        auto output = std::make_unique<MomentRulebook>(system, input.infer_from_factors);
        if (!input.human_readable_name.empty()) {
            output->set_name(input.human_readable_name);
        }

        // Import rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_op_seq_polynomials.size());
        for (auto& raw_rule : input.raw_op_seq_polynomials) {
            raw_polynomials.emplace_back(raw_rule->to_polynomial(output->factory));
        }
        output->add_raw_rules(std::move(raw_polynomials));

        // Compile and return
        output->complete();
        return output;
    }
}