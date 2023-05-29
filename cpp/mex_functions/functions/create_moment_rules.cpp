/**
 * create_moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "create_moment_rules.h"

#include "storage_manager.h"

#include "scenarios/context.h"

#include "symbolic/order_symbols_by_hash.h"
#include "symbolic/moment_substitution_rulebook.h"
#include "symbolic/symbol_table.h"

#include "export/export_moment_substitution_rules.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/read_choice.h"

namespace Moment::mex::functions {

    class OpSeqRuleSpecification {
    public:
    public:
        /** One input operator sequence with factor */
        class OpSeqExpr {
        public:
            sequence_storage_t raw_sequence;
            std::complex<double> factor;
            std::optional<OperatorSequence> resolved_sequence;
            symbol_name_t symbol_id = 0;
            bool conjugated = false;

    public:
            void raw_to_resolved(matlab::engine::MATLABEngine& engine,
                                const size_t rule_idx, const size_t elem_idx,
                                const Context& context) {
                const size_t op_count = context.size();
                for (size_t seq_idx = 0; seq_idx < this->raw_sequence.size(); ++seq_idx) {
                    const oper_name_t op = this->raw_sequence[seq_idx];
                    if (op >= op_count) {
                        std::stringstream errSS;
                        errSS << "Operator '" << op << "' in rule #" << (rule_idx+1)
                              << ", element #" << (elem_idx+1) << ", position #" << (seq_idx+1) << " is out of range.";
                        throw_error(engine, errors::bad_param, errSS.str());
                    }
                }
                this->resolved_sequence.emplace(std::move(this->raw_sequence), context);
            }

            void look_up_symbol(matlab::engine::MATLABEngine& engine,
                                const size_t rule_idx, const size_t elem_idx,
                                const SymbolTable& symbols) {
                assert(this->resolved_sequence.has_value());
                auto [where, is_cc] = symbols.where_and_is_conjugated(this->resolved_sequence.value());

                if (where == nullptr) {
                    std::stringstream errSS;
                    errSS << "Sequence \"" << this->resolved_sequence.value().formatted_string() << "\""
                          <<  " in rule #" << (rule_idx+1) << ", element #" << (elem_idx+1)
                          << " does not correspond to a known symbol, and automatic creation was disabled.";
                    throw_error(engine, errors::bad_param, errSS.str());
                }
                this->symbol_id = where->Id();
                this->conjugated = is_cc;
            }

            void look_up_or_make_symbol(matlab::engine::MATLABEngine& engine,
                                const size_t rule_idx, const size_t elem_idx,
                                SymbolTable& symbols) {
                assert(this->resolved_sequence.has_value());
                auto [where, is_cc] = symbols.where_and_is_conjugated(this->resolved_sequence.value());
                if (where != nullptr) {
                    this->symbol_id = where->Id();
                    this->conjugated = is_cc;
                } else {
                    this->symbol_id = symbols.merge_in(OperatorSequence{this->resolved_sequence.value()});
                    this->conjugated = false;
                }
            }
        };

        /** One input rule */
        class OpSeqRule {
        public:
            std::vector<OpSeqExpr> raw_elements;
            Polynomial::storage_t resolved_symbols;

            void make_resolved_symbols() {
                resolved_symbols.reserve(raw_elements.size());
                for (const auto& elem : raw_elements) {
                    resolved_symbols.emplace_back(elem.symbol_id, elem.factor, elem.conjugated);
                }
            }

            [[nodiscard]] Polynomial to_symbol_combo(const PolynomialFactory& factory) {
                return factory(std::move(this->resolved_symbols));
            }
        };

        std::vector<OpSeqRule> data;

        /**
         * Parse 'raw sequences' into OperatorSequence objects, associated with target context.
         */
        void contextualize_op_seqs(matlab::engine::MATLABEngine& engine,
                                   MatrixSystem& system) {
            const auto& context = system.Context();
            const size_t op_count = context.size();

            size_t rule_idx = 0;
            for (auto& raw_rule : this->data) {
                size_t elem_idx = 0;
                for (auto& raw_elem : raw_rule.raw_elements) {
                    raw_elem.raw_to_resolved(engine, rule_idx, elem_idx, context);
                }
            }
        }

        /**
         * Find associated symbol with every operator sequence.
         */
        void look_up_symbols(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols) {
            size_t rule_idx = 0;
            for (auto& raw_rule : this->data) {
                size_t elem_idx = 0;
                for (auto& raw_elem : raw_rule.raw_elements) {
                    raw_elem.look_up_symbol(engine, rule_idx, elem_idx, symbols);
                }
                raw_rule.make_resolved_symbols();
            }
        }

        /**
         * Find associated symbol with every operator sequence. Make it, if it doesn't exist already.
         */
        void look_up_or_make_symbols(matlab::engine::MATLABEngine& engine, SymbolTable& symbols) {
            size_t rule_idx = 0;
            for (auto& raw_rule : this->data) {
                size_t elem_idx = 0;
                for (auto& raw_elem : raw_rule.raw_elements) {
                    raw_elem.look_up_or_make_symbol(engine, rule_idx, elem_idx, symbols);
                }
                raw_rule.make_resolved_symbols();
            }
        }

    };

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

        // Do we automatically register new symbols, if they are specified.
        if (this->flags.contains(u"no_new_symbols")) {
            this->create_missing_symbols = false;
        }

        // Merge into existing rule-set?
        auto rulebook_param_iter = this->params.find(u"rulebook");
        if (rulebook_param_iter != this->params.cend()) {
            this->existing_rule_key = read_as_uint64(this->matlabEngine, rulebook_param_iter->second);
            this->merge_into_existing = true;
        } else {
            this->merge_into_existing = false;
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
                throw_error(this->matlabEngine, errors::bad_param, "Unknown input mode.");
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
            this->raw_symbol_polynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine,
                                                                               ruleNameSS.str(),
                                                                               cell_input[index]));
        }
    }

    void CreateMomentRulesParams::parse_as_op_seq_polynomials(const matlab::data::Array& input) {
        this->raw_op_seq_polynomials = std::make_unique<OpSeqRuleSpecification>();

        // Empty input can be interpreted as empty substitution list, and so is valid
        if (input.isEmpty()) {
            return;
        }

        // Input must be cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << "Operator polynomial list should be provided as a cell array.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        // Cast input to cell array [matlab says this kind of cast produces references, not copies. We can only hope...]
        const auto cell_input = static_cast<const matlab::data::CellArray>(input);
        const size_t rule_count = cell_input.getNumberOfElements();
        this->raw_symbol_polynomials.reserve(rule_count);

        // Go through elements
        for (size_t rule_index = 0; rule_index < rule_count; ++rule_index) {

            // Each element must be a cell array
            if (cell_input[rule_index].getType() != matlab::data::ArrayType::CELL) {
                std::stringstream errSS;
                errSS << "Rule #" << (rule_index+1) << " must be a cell array.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }

            const auto polynomial_cell = static_cast<const matlab::data::CellArray>(cell_input[rule_index]);
            const size_t polynomial_size = polynomial_cell.getNumberOfElements();

            this->raw_op_seq_polynomials->data.emplace_back();
            auto& parsed_polynomial = this->raw_op_seq_polynomials->data.back().raw_elements;
            parsed_polynomial.reserve(polynomial_size);

            // Go through elements in cell
            for (size_t elem_index = 0; elem_index < polynomial_size; ++elem_index) {

                // Check symbol expression is cell
                if (polynomial_cell[elem_index].getType() != matlab::data::ArrayType::CELL) {
                    std::stringstream errSS;
                    errSS << "Rule #" << (rule_index+1) << " element #" << (elem_index+1) << " must be a cell array.";
                    throw_error(this->matlabEngine, errors::bad_param, errSS.str());
                }

                // Check symbol expression cell has 1 or 2 elements
                const auto symbol_expr_cell = static_cast<const matlab::data::CellArray>(polynomial_cell[elem_index]);
                size_t symbol_expr_size = symbol_expr_cell.getNumberOfElements();
                if ((symbol_expr_size < 1) || (symbol_expr_size > 2)) {
                    std::stringstream errSS;
                    errSS << "Rule #" << (rule_index+1) << " element #" << (elem_index+1) << " must be a cell array "
                          << "containing an operator sequence and optionally a factor.";
                    throw_error(this->matlabEngine, errors::bad_param, errSS.str());
                }

                // Finally, attempt to read operators
                try {
                    // Read op sequence, and translate
                    std::vector<oper_name_t> raw_vec = read_as_vector<oper_name_t>(this->matlabEngine,
                                                                                   symbol_expr_cell[0]);
                    parsed_polynomial.emplace_back();
                    auto& expr = parsed_polynomial.back();
                    expr.raw_sequence.reserve(raw_vec.size());
                    size_t op_index = 0;
                    for (auto op : raw_vec) {
                        if (op < 1) {
                            std::stringstream errSS;
                            errSS << "Operator '" << op << "' at position #"
                                  << (op_index+1) << " is out of range.";
                            throw std::range_error{errSS.str()};
                        }
                        expr.raw_sequence.emplace_back(op-1);
                        ++op_index;
                    }

                    // Read factor
                    if (symbol_expr_size == 2) {
                        expr.factor = read_as_complex_scalar<double>(this->matlabEngine, symbol_expr_cell[1]);
                    } else {
                        expr.factor = 1.0;
                    }
                } catch (const std::exception& e) {
                    std::stringstream errSS;
                    errSS << "Error reading rule #" << (rule_index+1) << " element #" << (elem_index+1) << ": " << e.what();
                    throw_error(this->matlabEngine, errors::bad_param, errSS.str());
                }
            }
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
        this->param_names.emplace(u"rulebook");

        this->flag_names.emplace(u"no_factors");
        this->flag_names.emplace(u"no_new_symbols");
        this->flag_names.emplace(u"complete_only");
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

        // Create rule-book with new rules
        auto rulebookPtr = this->create_rulebook(system, input);

        // Add or merge rulebooks
        auto [rb_id, rulebook] = [&]() -> std::pair<size_t, MomentSubstitutionRulebook&> {
            if (input.merge_into_existing) {
                return system.merge_rulebooks(input.existing_rule_key, std::move(*rulebookPtr));
            } else {
                // Register rulebook with matrix system
                return system.create_rulebook(std::move(rulebookPtr));
            };
        }();


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


    std::unique_ptr<PolynomialFactory>
    CreateMomentRules::make_factory(SymbolTable& symbols, const CreateMomentRulesParams &input) const {
        switch (input.ordering) {
            case CreateMomentRulesParams::SymbolOrdering::ById:
                return std::make_unique<PolynomialFactory>(symbols);
            case CreateMomentRulesParams::SymbolOrdering::ByOperatorHash:
                return std::make_unique<ByHashPolynomialFactory>(symbols);
            default:
            case CreateMomentRulesParams::SymbolOrdering::Unknown:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown symbol ordering type.");
        }
    }

    std::unique_ptr<MomentSubstitutionRulebook>
    CreateMomentRules::create_rulebook(MatrixSystem& system, CreateMomentRulesParams& input) const {
        std::unique_ptr<MomentSubstitutionRulebook> book;
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
                throw_error(this->matlabEngine, errors::internal_error, "Unknown rules input mode.");
        }
        assert(book);

        // Extra rules from factors (if any)
        if (input.infer_from_factors) {
            book->infer_additional_rules_from_factors(system);
        }

        return book;
    }


    std::unique_ptr<MomentSubstitutionRulebook>
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
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
            ++idx;
        }
        // Make empty rulebook
        auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, this->make_factory(symbols, input));

        // Import rules, and compile
        output->add_raw_rules(input.sub_list);
        output->complete();

        return output;
    }

    std::unique_ptr<MomentSubstitutionRulebook>
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
                    throw_error(this->matlabEngine, errors::bad_param, errSS.str());
                }
                ++elem_idx;
            }
            ++idx;
        }

        // Construct empty ruleset with ordering
        auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, this->make_factory(symbols, input));
        const auto& factory = output->Factory();

        // Read rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_symbol_polynomials.size());
        for (const auto& raw_rule : input.raw_symbol_polynomials) {
            raw_polynomials.emplace_back(raw_data_to_polynomial(factory, raw_rule));
        }

        // Import rules, and compile
        output->add_raw_rules(std::move(raw_polynomials));
        output->complete();

        return output;
    }


    std::unique_ptr<MomentSubstitutionRulebook>
    CreateMomentRules::create_rulebook_from_new_sequences(Moment::MatrixSystem &system,
                                                          CreateMomentRulesParams &input) const {
        auto write_lock = system.get_write_lock();
        auto& symbols = system.Symbols();
        input.raw_op_seq_polynomials->contextualize_op_seqs(this->matlabEngine, system);
        input.raw_op_seq_polynomials->look_up_or_make_symbols(this->matlabEngine, symbols);

        //Make empty rulebook and get factory...
        auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, this->make_factory(symbols, input));
        auto& factory = output->Factory();

        // Import rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_op_seq_polynomials->data.size());
        for (auto& raw_rule : input.raw_op_seq_polynomials->data) {
            raw_polynomials.emplace_back(raw_rule.to_symbol_combo(factory));
        }
        output->add_raw_rules(std::move(raw_polynomials));

        // Compile and return
        output->complete();
        return output;
    }

    std::unique_ptr<MomentSubstitutionRulebook>
    CreateMomentRules::create_rulebook_from_existing_sequences(MatrixSystem& system,
                                                               CreateMomentRulesParams& input) const {
        auto read_lock = system.get_read_lock();
        auto& symbols = system.Symbols();
        input.raw_op_seq_polynomials->contextualize_op_seqs(this->matlabEngine, system);
        input.raw_op_seq_polynomials->look_up_symbols(this->matlabEngine, symbols);

        //Make empty rulebook and get factory...
        auto output = std::make_unique<MomentSubstitutionRulebook>(symbols, this->make_factory(symbols, input));
        auto& factory = output->Factory();

        // Import rules
        std::vector<Polynomial> raw_polynomials;
        raw_polynomials.reserve(input.raw_op_seq_polynomials->data.size());
        for (auto& raw_rule : input.raw_op_seq_polynomials->data) {
            raw_polynomials.emplace_back(raw_rule.to_symbol_combo(factory));
        }
        output->add_raw_rules(std::move(raw_polynomials));

        // Compile and return
        output->complete();
        return output;
    }
}