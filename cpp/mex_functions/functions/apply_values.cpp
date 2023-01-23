/**
 * apply_values.cpp
 *
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "apply_values.h"
#include "storage_manager.h"

#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "symbolic/substitution_list.h"

namespace Moment::mex::functions  {


    void ApplyValuesParams::extra_parse_params(matlab::engine::MATLABEngine &matlabEngine) {
        assert(inputs.empty());

        // Get matrix index
        auto& index_param = this->find_or_throw(u"index");
        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Parameter 'index'", index_param, 0);

        // Parse substitution cells
        auto& sub_list_param = this->find_or_throw(u"substitutions");
        this->substitutions = read_substitution_cell(matlabEngine, "Parameter 'substitutions'", sub_list_param);

    }

    void ApplyValuesParams::extra_parse_inputs(matlab::engine::MATLABEngine &matlabEngine) {
        assert(inputs.size() == 3);

        this->matrix_index = read_positive_integer<size_t>(matlabEngine, "Matrix index", inputs[1], 0);
        this->substitutions = read_substitution_cell(matlabEngine, "Substitution list", inputs[2]);
    }

    bool ApplyValuesParams::any_param_set() const {
        return this->params.contains(u"index")
            || this->params.contains(u"substitutions")
            || OperatorMatrixParams::any_param_set();
    }

    std::map<symbol_name_t, double>
    ApplyValuesParams::read_substitution_cell(matlab::engine::MATLABEngine &engine,
                                              const std::string &param_str,
                                              const matlab::data::Array &input) {
        // Empty input interpreted as empty substitution list
        if (input.isEmpty()) {
            return {};
        }

        // Input must be cell
        if (input.getType() != matlab::data::ArrayType::CELL) {
            std::stringstream errSS;
            errSS << param_str << " should be provided as a cell array.";
            throw_error(engine, errors::bad_param, errSS.str());
        }

        // Cast input to cell array
        const auto cell_input = static_cast<matlab::data::CellArray>(input);
        const size_t sub_count = cell_input.getNumberOfElements();

        // Read through cell array, and build output
        std::map<symbol_name_t, double> output;
        for (size_t index = 0; index < sub_count; ++index) {
            const auto the_cell = cell_input[index];
            if (the_cell.getType() != matlab::data::ArrayType::CELL) {
                std::stringstream errSS;
                errSS << param_str << " element " << (index+1) << " must be a cell array.";
            }
            auto the_cell_as_cell = static_cast<matlab::data::CellArray>(the_cell);
            if (the_cell_as_cell.getNumberOfElements() != 2) {
                std::stringstream errSS;
                errSS << param_str << " element " << (index+1) << " must have two elements: {symbol id, value}.";
                throw_error(engine, errors::bad_param, errSS.str());
            }
            auto symbol_id = read_as_scalar<symbol_name_t>(engine, the_cell_as_cell[0]);
            auto value = read_as_scalar<double>(engine, the_cell_as_cell[1]);

            // Cursory validation of symbol_id (must be non-negative, and not 0 or 1)
            if (symbol_id < 2) {
                std::stringstream  errSS;
                errSS << param_str << " element " << (index+1);
                if (symbol_id < 0) {
                    errSS << " cannot be negative.";
                } else {
                    errSS << " cannot bind reserved symbol \"" << symbol_id << "\".";
                }
                throw_error(engine, errors::bad_param, errSS.str());
            }

            output.emplace(std::pair(symbol_id, value));
        }

        return output;
    }

    ApplyValues::ApplyValues(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : OperatorMatrix{matlabEngine, storage, u"apply_values"} {
        this->param_names.emplace(u"substitutions");
        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    ApplyValues::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &omp) {
        const auto& avp = dynamic_cast<const ApplyValuesParams&>(omp);

        // Lock symbol table to do preprocessing of substitution list
        auto read_lock = system.get_read_lock();
        // Verify range of
        const auto max_symbols = system.Symbols().size();
        for (auto [key,val] : avp.substitutions) {
            if (key >= max_symbols) {
                std::stringstream errSS;
                errSS << "Cannot bind unknown symbol \"" << key << "\".";
                throw errors::BadInput{errors::bad_param, errSS.str()};
            }
        }
        // Preprocess substitution list
        SubstitutionList sub_list{std::move(avp.substitutions)};
        sub_list.infer_substitutions(system);
        read_lock.unlock();

        return system.clone_and_substitute(avp.matrix_index, sub_list);
    }
}