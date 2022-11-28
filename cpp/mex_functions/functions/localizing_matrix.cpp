/**
 * localizing_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "localizing_matrix.h"

#include "storage_manager.h"

#include "operators/matrix/localizing_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "fragments/export_operator_matrix.h"
#include "fragments/export_symbol_table.h"

#include <memory>


namespace NPATK::mex::functions {

    namespace {
        void offsetWordByMATLABIndices(matlab::engine::MATLABEngine& matlabEngine,
                                       std::vector<oper_name_t>& word,
                                       const LocalizingMatrixParams& lmp) {
            // Only apply offset if flag is set
            if (!lmp.flags.contains(u"matlab_indexing")) {
                return;
            }


        }
    }

    void LocalizingMatrixParams::extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) {
        assert(inputs.empty()); // Should be guaranteed by parent.

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get localizing word sequence
        auto& word_param = this->find_or_throw(u"word");
        this->localizing_word = read_integer_array(matlabEngine, "Parameter 'word'", word_param);

        // Do we offset by -1?
        this->matlab_indexing = this->flags.contains(u"matlab_indexing");
    }

    void LocalizingMatrixParams::extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) {
        // No named parameters... try to interpret inputs as matrix system, depth and word.
        assert(this->inputs.size() == 3); // should be guaranteed by parent.
        this->hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level", inputs[1], 0);
        this->localizing_word = read_integer_array(matlabEngine, "Localizing word", inputs[2]);

        // Do we offset by -1?
        this->matlab_indexing = this->flags.contains(u"matlab_indexing");

    }

    LocalizingMatrixIndex LocalizingMatrixParams::to_index(matlab::engine::MATLABEngine &matlabEngine,
                                                           const Context& context) const {
        // Do we have to offset?
        auto oper_copy = this->localizing_word;
        if (this->matlab_indexing) {
            for (auto &o: oper_copy) {
                // Throwing an error if any operator goes out of range
                if (0 == o) {
                    throw_error(matlabEngine, errors::bad_param,
                                "Operator with index 0 in localizing word is out of range.");
                }
                --o;
            }
        }

        // Check word is in range
        for (const auto op : oper_copy) {
            if (op >= context.size()) {
                throw_error(matlabEngine, errors::bad_param,
                            "Operator with index " + std::to_string(op) + " in localizing word is out of range.");
            }
        }

        // Copy and construct LMI
        return LocalizingMatrixIndex{context, this->hierarchy_level, OperatorSequence{std::move(oper_copy), context}};
    }

    bool LocalizingMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    LocalizingMatrix::LocalizingMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : NPATK::mex::functions::OperatorMatrix(matlabEngine, storage,
                                                    MEXEntryPointID::LocalizingMatrix, u"localizing_matrix") {
        // Either [ref, level, word] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        this->flag_names.emplace(u"matlab_indexing");

        this->max_inputs = 3;
    }

    std::unique_ptr<SortedInputs>
    LocalizingMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<LocalizingMatrixParams>(this->matlabEngine, std::move(input));
        output->parse(this->matlabEngine);

        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }

        return output;
    }

    std::pair<size_t, const NPATK::OperatorMatrix &>
    LocalizingMatrix::get_or_make_matrix(MatrixSystem &system, const OperatorMatrixParams& inputOMP) {
        const auto &input = dynamic_cast<const LocalizingMatrixParams&>(inputOMP);

        // Encode index under read lock
        auto read_lock = system.get_read_lock();
        auto lmi = input.to_index(this->matlabEngine, system.Context());
        read_lock.unlock();

        return system.create_localizing_matrix(lmi);
    }
}