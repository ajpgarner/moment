/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "storage_manager.h"

#include "matrix/operator_matrix/localizing_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "export/export_operator_matrix_seq_strings.h"
#include "export/export_symbol_table.h"

#include <memory>


namespace Moment::mex::functions {

    void LocalizingMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent.

        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->matlab_indexing = true;
        } else if (this->flags.contains(u"zero_indexing")) {
            this->matlab_indexing = false;
        }

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Parameter 'level'", depth_param, 0);

        // Get localizing word sequence
        auto& word_param = this->find_or_throw(u"word");
        this->localizing_word = read_integer_array<oper_name_t>(matlabEngine, "Parameter 'word'", word_param);

    }

    void LocalizingMatrixParams::extra_parse_inputs() {
        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->matlab_indexing = true;
        } else if (this->flags.contains(u"zero_indexing")) {
            this->matlab_indexing = false;
        }

        // No named parameters... try to interpret inputs as matrix system, depth and word.
        assert(this->inputs.size() == 3); // should be guaranteed by parent.
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy level", inputs[1], 0);
        this->localizing_word = read_integer_array<oper_name_t>(matlabEngine, "Localizing word", inputs[2]);

    }

    LocalizingMatrixIndex LocalizingMatrixParams::to_index(const Context& context) const {
        // Do we have to offset?
        sequence_storage_t oper_copy{};
        oper_copy.reserve(this->localizing_word.size());
        for (auto o : this->localizing_word) {
            if (this->matlab_indexing) {
                if (0 == o) {
                    throw_error(matlabEngine, errors::bad_param,
                                "Operator with index 0 in localizing word is out of range.");
                }
                o -= 1;
            }

            // Check in range
            if ((o < 0) || (o >= context.size())) {
                std::stringstream errSS;
                errSS << "Operator " << (this->matlab_indexing ? o + 1 : o) << " at index ";
                errSS << (oper_copy.size() + 1);
                errSS << " is out of range.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }
            oper_copy.emplace_back(o);
        }

        // Copy and construct LMI
        return LocalizingMatrixIndex{this->hierarchy_level, OperatorSequence{std::move(oper_copy), context}};
    }

    bool LocalizingMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    LocalizingMatrix::LocalizingMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, word] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");

        this->flag_names.emplace(u"zero_indexing");
        this->flag_names.emplace(u"matlab_indexing");
        this->mutex_params.add_mutex(u"zero_indexing", u"matlab_indexing");


        this->max_inputs = 3;
    }

    std::pair<size_t, const Moment::Matrix &>
    LocalizingMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        const auto &input = dynamic_cast<const LocalizingMatrixParams&>(inputOMP);

        // Encode index under read lock
        auto read_lock = system.get_read_lock();
        auto lmi = input.to_index(system.Context());
        read_lock.unlock();

        return system.LocalizingMatrix.create(lmi);
    }
}