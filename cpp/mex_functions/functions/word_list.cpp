/**
 * word_list.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "word_list.h"

#include "scenarios/context.h"
#include "dictionary/dictionary.h"

#include "storage_manager.h"

#include "export/export_operator_sequence.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    WordListParams::WordListParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {
        this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                            this->inputs[0], 0);


        this->word_length = read_positive_integer<uint64_t>(matlabEngine, "Word length",
                                                            this->inputs[1], 0);

        if (this->flags.contains(u"register_symbols")) {
            this->register_symbols = true;
        }
    }

    WordList::WordList(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_outputs = this->max_outputs = 1;
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->flag_names.insert(u"register_symbols");

    }

    void WordList::extra_input_checks(WordListParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
    }

    void WordList::operator()(IOArgumentRange output, WordListParams &input) {
        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const Moment::errors::persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        if (input.register_symbols) {
            matrixSystemPtr->generate_dictionary(input.word_length);
        }

        // Get read lock on system
        std::shared_lock lock = matrixSystemPtr->get_read_lock();

        // Get context
        const auto &dictionary = matrixSystemPtr->Context().osg_list();

        // Get (or make) unique word list.
        const auto &osg = dictionary[input.word_length];


        // Output list of parsed rules
        if (output.size() >= 1) {
            matlab::data::ArrayFactory factory;
            output[0] = export_all_operator_sequences(factory, osg, true);
        }
    }
}