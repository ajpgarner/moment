/**
 * create_moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "create_moment_rules.h"

#include "storage_manager.h"

#include "symbolic/order_symbols_by_hash.h"
#include "symbolic/moment_substitution_rulebook.h"
#include "symbolic/symbol_table.h"

#include "export/export_moment_substitution_rules.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/read_choice.h"

namespace Moment::mex::functions {

    namespace {


        std::unique_ptr<SymbolComboFactory> make_factory(matlab::engine::MATLABEngine& engine,
                                                         SymbolTable& symbols,
                                                         CreateMomentRulesParams::SymbolOrdering order) {
            switch (order) {
                case CreateMomentRulesParams::SymbolOrdering::ById:
                    return std::make_unique<SymbolComboFactory>(symbols);
                case CreateMomentRulesParams::SymbolOrdering::ByOperatorHash:
                    return std::make_unique<ByHashSymbolComboFactory>(symbols);
                default:
                case CreateMomentRulesParams::SymbolOrdering::Unknown:
                    throw_error(engine, errors::internal_error, "Unknown symbol ordering type.");
            }
        }


        std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_sublist(matlab::engine::MATLABEngine& engine,
                                     SymbolTable& symbols, const CreateMomentRulesParams& input) {
            // Range check sublist vs. symbol table
            size_t idx = 0;
            for (auto [id, val] : input.sub_list) {
                if (id >= symbols.size()) {
                    std::stringstream errSS;
                    errSS << "Symbol " << id << " not found (substitution list element " << (idx+1) << ")";
                    throw_error(engine, errors::bad_param, errSS.str());
                }
                ++idx;
            }

            // Construct ordering
            std::unique_ptr<SymbolComboFactory> factory = make_factory(engine, symbols, input.ordering);

            // Make empty rulebook
            auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, std::move(factory));

            // Import rules, and compile
            output->add_raw_rules(input.sub_list);
            output->complete();

            return output;
        }

        std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_symbols(matlab::engine::MATLABEngine& engine,
                                     SymbolTable& symbols, const CreateMomentRulesParams& input) {

            // Construct ordering
            std::unique_ptr<SymbolComboFactory> factory = make_factory(engine, symbols, input.ordering);

            // Range check data vs. symbol table
            size_t idx = 0;
            for (const auto& rule : input.raw_symbol_polynomials) {
                size_t elem_idx = 0;
                for (const auto& elem : rule) {
                    if (elem.symbol_id >= symbols.size()) {
                        std::stringstream errSS;
                        errSS << "Symbol " << elem.symbol_id << " not found "
                              << "(rule #" << (idx+1) << ",  element " << (elem_idx+1) << ").";
                        throw_error(engine, errors::bad_param, errSS.str());
                    }
                    ++elem_idx;
                }
                ++idx;
            }

            // Read rules
            std::vector<SymbolCombo> raw_polynomials;
            raw_polynomials.reserve(input.raw_symbol_polynomials.size());
            for (const auto& raw_rule : input.raw_symbol_polynomials) {
                raw_polynomials.emplace_back(raw_sc_data_to_symbol_combo(*factory, raw_rule));
            }

            //Make empty rulebook
            auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, std::move(factory));

            // Import rules, and compile
            output->add_raw_rules(std::move(raw_polynomials));
            output->complete();

            return output;
        }

        std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_sequences(matlab::engine::MATLABEngine& engine,
                                       SymbolTable& symbols, const CreateMomentRulesParams& input) {
            throw_error(engine, errors::internal_error, "create_rulebook_from_sequences not implemented.");
        }

        std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook(matlab::engine::MATLABEngine& engine,
                        MatrixSystem& system, const CreateMomentRulesParams& input) {

            std::unique_ptr<MomentSubstitutionRulebook> book;
            switch (input.input_mode) {
                case CreateMomentRulesParams::InputMode::SubstitutionList:
                    book = create_rulebook_from_sublist(engine, system.Symbols(), input);
                    break;
                case CreateMomentRulesParams::InputMode::FromSymbolIds:
                    book = create_rulebook_from_symbols(engine, system.Symbols(), input);
                    break;
                case CreateMomentRulesParams::InputMode::FromOperatorSequences:
                    book = create_rulebook_from_sequences(engine, system.Symbols(), input);
                    break;
                default:
                case CreateMomentRulesParams::InputMode::Unknown:
                    throw_error(engine, errors::internal_error, "Unknown rules input mode.");
            }

            // Extra rules from factors (if any)
            if (input.infer_from_factors) {
                book->infer_additional_rules_from_factors(system);
            }

            return book;
        }
    }

    CreateMomentRulesParams::CreateMomentRulesParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {
        // Read matrix key
        this->matrix_system_key = read_positive_integer<size_t>(matlabEngine, "Matrix system reference", inputs[0], 0);

        // Ascertain input mode
        if (this->flags.contains(u"list")) {
            this->input_mode = InputMode::SubstitutionList;
        } else if (this->flags.contains(u"symbols")) {
            this->input_mode = InputMode::FromSymbolIds;
        } else if (this->flags.contains(u"sequences")) {
            this->input_mode = InputMode::FromOperatorSequences;
        }

        // Ascertain symbol ordering
        auto order_param_iter = this->params.find(u"order");
        if (order_param_iter != this->params.cend()) {
            try {
                switch (read_choice("Parameter 'order'", {"id", "hash"}, order_param_iter->second)) {
                    case 0:
                        this->ordering = SymbolOrdering::ById;
                        break;
                    case 1:
                        this->ordering = SymbolOrdering::ByOperatorHash;
                        break;
                    default:
                        this->ordering = SymbolOrdering::Unknown;
                        break;
                }
            } catch (const Moment::mex::errors::invalid_choice& ice) {
                throw_error(this->matlabEngine, errors::bad_param, ice.what());
            }
        }

        // Do we automatically add rules arising from factorization?
        if (this->flags.contains(u"no_factors")) {
            this->infer_from_factors = false;
        }

        // Extra checks?
        switch (this->input_mode) {
            case InputMode::SubstitutionList:
                parse_as_sublist(this->inputs[1]);
                break;
            case InputMode::FromSymbolIds:
                parse_as_symbol_polynomials(this->inputs[1]);
                break;
            default:
                break;
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
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
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
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
            const matlab::data::CellArray the_cell_as_cell = the_cell;
            if (the_cell_as_cell.getNumberOfElements() != 2) {
                std::stringstream errSS;
                errSS << "Substitution list element " << (index+1) << " must have two elements: {symbol id, value}.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
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
                throw_error(matlabEngine, errors::bad_param, errSS.str());
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
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        // Cast input to cell array [matlab says this will be a reference, not copy. We can hope...]
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t sub_count = cell_input.getNumberOfElements();
        this->raw_symbol_polynomials.reserve(sub_count);

        // Go through elements
        for (size_t index = 0; index < sub_count; ++index) {
            std::stringstream ruleNameSS;
            ruleNameSS << "Rule #" << (index+1);
            this->raw_symbol_polynomials.emplace_back(read_raw_symbol_combo_data(this->matlabEngine,
                                                                                 ruleNameSS.str(),
                                                                                 cell_input[index]));
        }
    }

    CreateMomentRules::CreateMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"create_moment_rules") {
        this->min_inputs = 2;
        this->max_inputs = 2;

        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"list");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->mutex_params.add_mutex({u"list", u"symbols", u"sequences"});

        this->param_names.emplace(u"order");

        this->flag_names.emplace(u"no_factors");
    }

    void CreateMomentRules::extra_input_checks(CreateMomentRulesParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }

    void CreateMomentRules::operator()(IOArgumentRange output, CreateMomentRulesParams &input) {
        // Get stored moment matrix
        auto msPtr = [&]() -> std::shared_ptr<MatrixSystem> {
            try {
                return this->storageManager.MatrixSystems.get(input.matrix_system_key);
            } catch (const Moment::errors::not_found_error& nfe) {
                throw_error(this->matlabEngine, errors::bad_param,
                            std::string("Matrix system not found: ").append(nfe.what()));
            }
        }();
        auto& system = *msPtr;

        // Create rule-book (with read-lock on matrix system)
        auto read_lock = msPtr->get_read_lock();
        auto rulebookPtr = create_rulebook(this->matlabEngine, system, input);
        read_lock.unlock();

        // Register rulebook with matrix system
        auto [rb_id, rulebook] = system.create_rulebook(std::move(rulebookPtr));

        // Output rulebook ID
        matlab::data::ArrayFactory factory;
        if (output.size() >= 1) {
            output[0] = factory.createScalar<uint64_t>(rb_id);
        }

        // Output 'complete' rules
        if (output.size() >= 2) {
            auto new_read_lock = msPtr->get_read_lock();
            MomentSubstitutionRuleExporter msrExporter{this->matlabEngine, system.Symbols()};
            output[1] = msrExporter(rulebook);
        }
    }
}