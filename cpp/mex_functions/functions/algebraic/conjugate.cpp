/**
 * conjugate.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "conjugate.h"

#include "storage_manager.h"

#include "scenarios/context.h"

#include "export/export_operator_sequence.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include <sstream>

namespace Moment::mex::functions {

    ConjugateParams::ConjugateParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Read op string, translate from MATLAB to C++ indexing
        if (inputs[1].getType() == matlab::data::ArrayType::CELL) {
            const auto input_dims = inputs[1].getDimensions();
            this->input_shape.reserve(input_dims.size());
            std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

            const matlab::data::CellArray as_cell = inputs[1];
            this->operator_string.reserve(as_cell.getNumberOfElements());
            for (auto str : as_cell) {
                this->operator_string.emplace_back(
                        read_integer_array<oper_name_t>(matlabEngine, "Operator string", str));
                for (auto &op: this->operator_string.back()) {
                    if (op < 1) {
                        throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
                    }
                    op -= 1;
                }
            }
        } else {
            this->input_shape = {1, 1};
            this->operator_string.emplace_back(
                    read_integer_array<oper_name_t>(matlabEngine, "Operator string", inputs[1]));
            for (auto &op: this->operator_string.back()) {
                if (op < 1) {
                    throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
                }
                op -= 1;
            }
        }
    }

    bool ConjugateParams::scalar_input() const noexcept {
        return ((this->input_shape.size() == 2) && (this->input_shape[0] == 1) && (this->input_shape[1] == 1));
    }


    void Conjugate::extra_input_checks(ConjugateParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Conjugate::Conjugate(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;
    }

    void Conjugate::operator()(IOArgumentRange output, ConjugateParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto lock = matrixSystem.get_read_lock();
        const Context& context = matrixSystem.Context();

        if (input.scalar_input()) {
            assert(input.operator_string.size() == 1);

            this->validate_op_seq(context, input.operator_string[0], 0);
            sequence_storage_t rawOpStr{input.operator_string[0].begin(), input.operator_string[0].end()};
            OperatorSequence opSeq{std::move(rawOpStr), context};
            auto conjugateSeq = opSeq.conjugate();

            if (this->verbose) {
                std::stringstream ss;
                ss << opSeq << " -> " << conjugateSeq << "\n";
                print_to_console(matlabEngine, ss.str());
            }

            matlab::data::ArrayFactory factory;
            output[0] = export_operator_sequence(factory, conjugateSeq);

            // Export minus sign if necessary
            if (output.size() >= 2) {
                output[1] = factory.createScalar<std::complex<double>>(to_scalar(conjugateSeq.get_sign()));
            }

            // Export hash
            if (output.size() >= 3) {
                output[2] = factory.createScalar<uint64_t>(conjugateSeq.hash());
            }
        } else {

            matlab::data::ArrayFactory factory;

            // Prepare outputs
            auto out_op_seqs = factory.createCellArray(input.input_shape);
            auto out_sign = factory.createArray<std::complex<double>>(input.input_shape);
            auto out_hashes = factory.createArray<uint64_t>(input.input_shape);

            // Parse and conjugate
            size_t write_index = 0;
            auto out_op_seqs_iter = out_op_seqs.begin();
            auto out_sign_iter = out_sign.begin();
            auto out_hashes_iter = out_hashes.begin();

            std::stringstream ss;

            for (const auto& input_seq : input.operator_string) {
                this->validate_op_seq(context, input.operator_string[write_index], write_index+1);
                sequence_storage_t rawOpStr{input_seq.begin(), input_seq.end()};
                OperatorSequence opSeq{std::move(rawOpStr), context};
                auto conjugateSeq = opSeq.conjugate();
                if (this->verbose) {
                    ss << opSeq << " -> " << conjugateSeq << "\n";

                }

                *out_op_seqs_iter = export_operator_sequence(factory, conjugateSeq);
                *out_sign_iter = to_scalar(conjugateSeq.get_sign());
                *out_hashes_iter = conjugateSeq.hash();

                // Next:
                ++out_op_seqs_iter;
                ++out_sign_iter;
                ++out_hashes_iter;
                ++write_index;
            }

            if (this->verbose) {
                print_to_console(matlabEngine, ss.str());
            }

            // Move outputs
            output[0] = std::move(out_op_seqs);
            if (output.size() >= 2) {
                output[1] = std::move(out_sign);
            }
            if (output.size() >= 3) {
                output[2] = std::move(out_hashes);
            }
        }

    }

    void Conjugate::validate_op_seq(const Context& context, std::span<const oper_name_t> operator_string, size_t index) const {
        size_t idx = 1; // MATLAB 1-indexing
        for (auto op_num : operator_string) {
            if ((op_num < 0) || (op_num >= context.size())) {
                std::stringstream errSS;
                errSS << "Operator " << (op_num + 1) << " at index " << idx;
                if (index > 0) {
                    errSS << " of entry " << index;
                }
                errSS << " is out of range.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
            ++idx;
        }

    }

}
