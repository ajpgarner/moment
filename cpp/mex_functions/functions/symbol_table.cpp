/**
 * symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_table.h"
#include "storage_manager.h"

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
            this->output_mode = OutputMode::SearchBySequence;
            std::vector<uint64_t> raw_op_seq = read_positive_integer_array<uint64_t>(matlabEngine, "Operator sequence",
                                                                                     this->inputs[1], 1);
            this->sequence.reserve(raw_op_seq.size());
            for (auto ui : raw_op_seq) {
                this->sequence.emplace_back(ui - 1);
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
                for (auto x: this->sequence) {
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
                    output[0] = export_symbol_table_struct(this->matlabEngine, matrixSystem);
                    break;
                case SymbolTableParams::OutputMode::FromId:
                    output[0] = export_symbol_table_struct(this->matlabEngine, matrixSystem, input.from_id);
                    break;
                case SymbolTableParams::OutputMode::SearchBySequence:
                    find_and_return_symbol(output, input, matrixSystem);
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
        OperatorSequence trialSequence(sequence_storage_t(input.sequence.begin(), input.sequence.end()), context);
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
            output[0] = export_symbol_table_row(this->matlabEngine, system, unique);
        }
        if (output.size() >= 2) {
            output[1] = factory.createScalar<bool>(conjugated);
        }


    }


}