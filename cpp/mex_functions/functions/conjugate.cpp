/**
 * transpose.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "conjugate.h"

#include "storage_manager.h"

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
        this->operator_string =  read_integer_array<oper_name_t>(matlabEngine, "Operator string", inputs[1]);
        for (auto& op : this->operator_string) {
            if (op < 1) {
                throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
            }
            op -= 1;
        }

    }


    void Conjugate::extra_input_checks(ConjugateParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Conjugate::Conjugate(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"conjugate") {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
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

        sequence_storage_t rawOpStr{input.operator_string.begin(), input.operator_string.end()};
        OperatorSequence opSeq{std::move(rawOpStr), context};
        auto conjugateSeq = opSeq.conjugate();

        if (this->verbose) {
            std::stringstream ss;
            ss << opSeq << " -> " << conjugateSeq << "\n";
            print_to_console(matlabEngine, ss.str());
        }

        matlab::data::ArrayFactory factory;
        output[0] = export_operator_sequence(factory, conjugateSeq);

    }

}
