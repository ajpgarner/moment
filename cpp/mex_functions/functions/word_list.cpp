/**
 * word_list.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "word_list.h"

#include "scenarios/context.h"
#include "scenarios/pauli/pauli_dictionary.h"
#include "dictionary/dictionary.h"

#include "storage_manager.h"

#include "export/export_osg.h"


#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    WordListParams::WordListParams(SortedInputs &&rawInput)
            : SortedInputs{std::move(rawInput)}, matrix_system_key{matlabEngine} {

        this->matrix_system_key.parse_input(this->inputs[0]);

        this->word_length = read_positive_integer<size_t>(matlabEngine, "Word length",
                                                          this->inputs[1], 0);

        // Optional parameters for nearest-neighbour mode
        this->find_and_parse(u"neighbours", [this](const matlab::data::Array& param) {
            this->extra_data.nearest_neighbours
                = read_positive_integer<uint64_t>(matlabEngine, "Parameter 'neighbours'", param, 0);
        });

        if (this->flags.contains(u"register_symbols")) {
            this->register_symbols = true;
        }

        if (this->flags.contains(u"monomial")) {
            if (this->register_symbols) {
                this->output_type = OutputType::FullMonomial;
            } else {
                this->output_type = OutputType::Monomial;
            }
        } else if (this->flags.contains(u"operators")) {
            this->output_type = OutputType::OperatorCell;
        }
    }

    WordList::WordList(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 7;
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->flag_names.insert(u"register_symbols");
        this->flag_names.insert(u"operators");
        this->flag_names.insert(u"monomial");

        this->param_names.emplace(u"neighbours");

        this->mutex_params.add_mutex(u"operators", u"monomial");
    }

    namespace {
        const OperatorSequenceGenerator& query_for_osg(matlab::engine::MATLABEngine& engine,
                                                       const Dictionary& dictionary, const WordListParams& params) {
            if (params.extra_data.nearest_neighbours != 0) {
                const auto* pauli_dict_ptr = dynamic_cast<const Pauli::PauliDictionary*>(&dictionary);
                if (nullptr == pauli_dict_ptr) {
                    throw_error(engine, errors::bad_param, "Only Pauli scenarios support nearest neighbours.");
                }

                Pauli::NearestNeighbourIndex nni{params.word_length, params.extra_data.nearest_neighbours};
                return pauli_dict_ptr->NearestNeighbour(nni)();
            }

            return dictionary.Level(params.word_length)();
        }
    }

    void WordList::operator()(IOArgumentRange output, WordListParams &input) {
        // Check output length
        switch (input.output_type) {
            case WordListParams::OutputType::OperatorCell:
                if (output.size() != 1) {
                    throw_error(this->matlabEngine, errors::too_many_outputs,
                                "Operators cell export expects one output.");
                }
                break;
            case WordListParams::OutputType::Monomial:
                if (output.size() != 3) {
                    throw_error(this->matlabEngine,
                                output.size() > 3 ? errors::too_many_outputs : errors::too_few_outputs,
                                "Monomial export expects three outputs.");
                }
                break;
            case WordListParams::OutputType::FullMonomial:
                if (output.size() != 7) {
                    throw_error(this->matlabEngine,
                                output.size() > 7 ? errors::too_many_outputs : errors::too_few_outputs,
                                "Full monomial export expects seven outputs.");
                }
                break;
        }

        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.matrix_system_key(this->storageManager);

        if (input.register_symbols) {
            matrixSystemPtr->generate_dictionary(input.word_length);
        }

        // Get read lock on system
        std::shared_lock lock = matrixSystemPtr->get_read_lock();

        // Get symbol table and dictionary
        const auto& symbols = matrixSystemPtr->Symbols();
        const auto& dictionary = matrixSystemPtr->Context().dictionary();

        // Get (or make) unique word list.
        const auto &osg = query_for_osg(this->matlabEngine, dictionary, input);

        // Output list of words
        OSGExporter exporter(this->matlabEngine, symbols);
        switch (input.output_type) {
            case WordListParams::OutputType::OperatorCell:
                output[0] = exporter.operators(osg, true);
                break;
            case WordListParams::OutputType::Monomial:
                exporter.sequences(output, osg);
                break;
            case WordListParams::OutputType::FullMonomial:
                exporter.sequences_with_symbol_info(output, osg);
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
        }
    }
}