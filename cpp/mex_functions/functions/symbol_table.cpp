/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_table.h"
#include "storage_manager.h"

#include "scenarios/context.h"

#include "symbolic/symbol_table.h"

#include "export/export_symbol_table.h"

#include "utilities/io_parameters.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {
    SymbolTableParams::SymbolTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {
        this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference", this->inputs[0], 0);

        auto fromIter = this->params.find(u"from");
        if (fromIter != this->params.end()) {
            this->from_id = read_positive_integer<symbol_name_t>(matlabEngine, "Symbol lower bound",
                                                                 fromIter->second, 0);
            this->output_mode = OutputMode::FromId;
            if (this->inputs.size() > 1) {
                throw_error(matlabEngine, errors::too_many_inputs,
                            "Only the MatrixSystem reference should be provided as input when \"from\" is used.");
            }
            return;
        }

        if (this->inputs.size() > 1) {
            if (this->inputs[1].getType() == matlab::data::ArrayType::CELL) {
                this->output_mode = OutputMode::SearchBySequenceArray;
                const matlab::data::CellArray query_inputs = this->inputs[1];
                const auto elems = query_inputs.getNumberOfElements();
                this->sequences.reserve(elems);

                for (size_t elem_index = 0; elem_index < elems; ++elem_index) {
                    const auto the_elem = query_inputs[elem_index];
                    std::vector<uint64_t> raw_op_seq = read_positive_integer_array<uint64_t>(matlabEngine,
                                                                                             "Operator sequence",
                                                                                             the_elem, 1);
                    this->sequences.emplace_back();
                    auto &seq = this->sequences.back();
                    seq.reserve(raw_op_seq.size());
                    for (auto ui: raw_op_seq) {
                        seq.emplace_back(ui - 1);
                    }
                }
            } else {
                this->output_mode = OutputMode::SearchBySequence;
                std::vector<uint64_t> raw_op_seq = read_positive_integer_array<uint64_t>(matlabEngine,
                                                                                         "Operator sequence",
                                                                                         this->inputs[1], 1);

                this->sequences.emplace_back();
                auto &seq = this->sequences.back();
                seq.reserve(raw_op_seq.size());
                for (auto ui: raw_op_seq) {
                    seq.emplace_back(ui - 1);
                }
            }
        } else {
            this->output_mode = OutputMode::AllSymbols;
        }
    }

    std::string SymbolTableParams::to_string() const {
        std::stringstream ss;
        ss << "Exporting symbol table from ref=" << this->storage_key << " ";
        switch (this->output_mode) {
            case SymbolTableParams::OutputMode::AllSymbols:
                ss << "in AllSymbols mode.\n";
                break;
            case SymbolTableParams::OutputMode::FromId:
                ss << "in FromId mode, with from=" << this->from_id << ".\n";
                break;
            case SymbolTableParams::OutputMode::SearchBySequence:
            {
                ss << "in SearchBySequence mode, with seq=";
                bool done_one = false;
                const auto& seq = this->sequences.front();
                for (auto x: seq) {
                    if (done_one) {
                        ss << ";";
                    }
                    done_one = true;
                    ss << x;
                }
                ss << ".\n";
            }
                break;
            default:
                ss << "in unknown mode.\n";
        }
        return ss.str();
    }


    SymbolTable::SymbolTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"symbol_table") {

        this->min_outputs = 1;
        this->max_outputs = 2;
        this->min_inputs = 1;
        this->max_inputs = 2;
        this->param_names.emplace(u"from");
    }



    void SymbolTable::extra_input_checks(SymbolTableParams &input) const {
        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(input.storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
    }


    void SymbolTable::validate_output_count(const size_t outputs, const SortedInputs &inputRaw) const {
        const auto& input = dynamic_cast<const SymbolTableParams&>(inputRaw);

        // Double check outputs
        if ((outputs >= 2) && (input.output_mode != SymbolTableParams::OutputMode::SearchBySequence)) {
            throw_error(this->matlabEngine, errors::too_many_outputs, "Too many outputs provided.");
        }
    }

    void SymbolTable::operator()(IOArgumentRange output, SymbolTableParams& input) {
        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }
        const auto& matrixSystem = *matrixSystemPtr;

        // Get read lock on system
        std::shared_lock lock = matrixSystem.get_read_lock();

        // Export symbol table
        if (output.size() >= 1) {
            switch (input.output_mode) {
                case SymbolTableParams::OutputMode::AllSymbols:
                    output[0] = export_symbol_table_struct(this->matlabEngine, *this->settings, matrixSystem);
                    break;
                case SymbolTableParams::OutputMode::FromId:
                    output[0] = export_symbol_table_struct(this->matlabEngine, *this->settings, matrixSystem, input.from_id);
                    break;
                case SymbolTableParams::OutputMode::SearchBySequence:
                    find_and_return_symbol(output, input, matrixSystem);
                    break;
                case SymbolTableParams::OutputMode::SearchBySequenceArray:
                    find_and_return_symbol_array(output, input, matrixSystem);
                    break;
                default:
                    throw_error(this->matlabEngine, errors::internal_error, "Unknown output mode.");

            }
        }

    }

    void SymbolTable::find_and_return_symbol(IOArgumentRange output, const SymbolTableParams& input,
                                const MatrixSystem &system) {
        matlab::data::ArrayFactory factory;

        const auto& context = system.Context();
        const auto& symbolTable = system.Symbols();

        // Try to find sequence
        const auto& seq = input.sequences.front();
        OperatorSequence trialSequence(sequence_storage_t(seq.begin(), seq.end()), context);
        const auto* symbolRow = symbolTable.where(trialSequence);

        // Return false if nothing found
        if (nullptr == symbolRow) {
            if (output.size() >= 1) {
                output[0] = factory.createScalar<bool>(false);
            }
            if (output.size() >= 2) {
                output[1] = factory.createScalar<bool>(false);
            }
            return;
        }

        // Otherwise, export row
        const auto& unique = *symbolRow;
        bool conjugated = trialSequence.hash() != unique.hash();
        assert(!conjugated || (trialSequence.hash() == unique.hash_conj()));

        if (output.size() >= 1) {
            output[0] = export_symbol_table_row(this->matlabEngine, *this->settings, system, unique);
        }
        if (output.size() >= 2) {
            output[1] = factory.createScalar<bool>(conjugated);
        }
    }


    void SymbolTable::find_and_return_symbol_array(IOArgumentRange output, const SymbolTableParams& input,
                                             const MatrixSystem &system) {
        // Do nothing if no outputs
        if (output.size() < 1) {
            return;
        }

        matlab::data::ArrayFactory factory;
        const auto& context = system.Context();
        const auto& symbolTable = system.Symbols();

        const size_t row_count = input.sequences.size();

        auto out_struct = factory.createStructArray(matlab::data::ArrayDimensions{1, row_count},
                                                    {"symbol", "operators", "conjugated"});

        for (size_t index = 0; index < row_count; ++ index) {
            const auto& seqRaw = input.sequences[index];
            OperatorSequence trialSequence(sequence_storage_t(seqRaw.begin(), seqRaw.end()), context);
            const auto symbolRow = symbolTable.where(trialSequence);

            if (symbolRow != nullptr) {
                const bool conjugated = trialSequence.hash() != symbolRow->hash();

                out_struct[index]["symbol"] = factory.createScalar<int64_t>(symbolRow->Id());
                out_struct[index]["operators"] = conjugated ? factory.createScalar(symbolRow->formatted_sequence_conj())
                                                           : factory.createScalar(symbolRow->formatted_sequence());
                out_struct[index]["conjugated"] = factory.createScalar<bool>(conjugated);

            } else {
                out_struct[index]["symbol"] = factory.createScalar<int64_t>(-1);
                out_struct[index]["operators"] = context.format_sequence(trialSequence);
                out_struct[index]["conjugated"] = factory.createScalar<bool>(false);
            }
        }
        output[0] = std::move(out_struct);
    }
}