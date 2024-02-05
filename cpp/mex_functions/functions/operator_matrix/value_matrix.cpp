/**
 * value_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "./value_matrix.h"

#include "utilities/read_as_string.h"

#include "matrix_system/matrix_system.h"
#include "matrix/symbolic_matrix.h"
#include "matrix/value_matrix.h"




namespace Moment::mex::functions  {

    ValueMatrixParams::ValueMatrixParams(SortedInputs&& raw_inputs)
        : OperatorMatrixParams{std::move(raw_inputs)}, raw_data{this->matlabEngine, "Data"} {
    }


    void ValueMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent

        this->find_and_parse_or_throw(u"data", [&](const auto& param_array) {
            this->load_numeric_array(param_array);
        });

        this->parse_optional_params();
    }

    void ValueMatrixParams::extra_parse_inputs() {
        // No named parameters... try to interpret inputs as Settings object + depth
        assert(this->inputs.size() == 2); // should be guaranteed by parent
        this->load_numeric_array(this->inputs[1]);

        this->parse_optional_params();
    }

    void ValueMatrixParams::parse_optional_params() {
        if (!this->find_and_parse(u"label", [&](const matlab::data::Array& label_array) {
            this->label = read_as_utf8(label_array);
        })) {
            this->label = std::nullopt;
        }
    }

    bool ValueMatrixParams::any_param_set() const {
        const bool data_specified = this->params.contains(u"data");
        return data_specified || OperatorMatrixParams::any_param_set();
    }

    void ValueMatrixParams::load_numeric_array(const matlab::data::Array& input) {
        // Ensure data is matrix
        if (input.getDimensions().size() != 2) {
            throw_error(this->matlabEngine, errors::bad_param, "Data was not a matrix.");
        }

        // Ensure data is numeric
        switch (input.getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::COMPLEX_SINGLE:
            case matlab::data::ArrayType::COMPLEX_DOUBLE:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
            case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE:
                break; // ok
            default:
                throw_error(this->matlabEngine, errors::bad_param, "Data was not numeric.");
        }

        // Load data
        this->raw_data.parse_input(input);

        // Sanity check
        switch (this->raw_data.type) {
            case AlgebraicOperand::InputType::RealNumber:
            case AlgebraicOperand::InputType::RealNumberArray:
            case AlgebraicOperand::InputType::ComplexNumber:
            case AlgebraicOperand::InputType::ComplexNumberArray:
                break; // ok
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Numeric data was not correctly parsed!");
        }

    }


    ValueMatrix::ValueMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"data");

        // Can tag data with a label
        this->param_names.emplace(u"label");

        this->max_inputs = 2;
    }


    std::pair<size_t, const Moment::SymbolicMatrix &>
    ValueMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        auto& input = dynamic_cast<ValueMatrixParams&>(inputOMP);

        // Construct matrix
        auto read_lock = system.get_read_lock();
        auto new_matrix_ptr = input.raw_data.to_value_matrix(system, input.label);
        read_lock.unlock();

        // Lock, then insert
        auto write_lock = system.get_write_lock();
        auto insertion_index = system.push_back(write_lock, std::move(new_matrix_ptr));
        return std::pair<size_t, const Moment::SymbolicMatrix&>{insertion_index, system.get(insertion_index)};
        // ~write_lock
    }


}