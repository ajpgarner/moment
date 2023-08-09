/**
 * symbol_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbol_table.h"
#include "storage_manager.h"

#include "scenarios/context.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "export/export_symbol_table.h"

#include "utilities/io_parameters.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {
    SymbolTableParams::SymbolTableParams(SortedInputs &&rawInput)
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

                auto input_iter = query_inputs.cbegin();
                while (input_iter != query_inputs.cend()) {
                //for (size_t elem_index = 0; elem_index < elems; ++elem_index) {
                    const matlab::data::Array the_elem = *input_iter;
                    std::vector<uint64_t> raw_op_seq = read_positive_integer_array<uint64_t>(matlabEngine,
                                                                                             "Operator sequence",
                                                                                             the_elem, 1);
                    this->sequences.emplace_back();
                    auto &seq = this->sequences.back();
                    seq.reserve(raw_op_seq.size());
                    for (auto ui: raw_op_seq) {
                        seq.emplace_back(static_cast<oper_name_t>(ui - 1));
                    }
                    ++input_iter;
                }

                const auto dims = query_inputs.getDimensions();
                this->sequence_dimensions.clear();
                std::copy(dims.cbegin(), dims.cend(), std::back_inserter(this->sequence_dimensions));


            } else {
                this->output_mode = OutputMode::SearchBySequence;
                std::vector<uint64_t> raw_op_seq = read_positive_integer_array<uint64_t>(matlabEngine,
                                                                                         "Operator sequence",
                                                                                         this->inputs[1], 1);

                this->sequences.emplace_back();
                auto &seq = this->sequences.back();
                seq.reserve(raw_op_seq.size());
                for (auto ui: raw_op_seq) {
                    seq.emplace_back(static_cast<oper_name_t>(ui - 1));
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
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;
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


    void SymbolTable::operator()(IOArgumentRange output, SymbolTableParams& input) {
        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const Moment::errors::persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }
        const auto& matrixSystem = *matrixSystemPtr;

        // Get read lock on system
        std::shared_lock lock = matrixSystem.get_read_lock();

        // Get appropriate exporter
        auto exporter = [&]() -> SymbolTableExporter {
            if (auto lmsPtr = dynamic_cast<const Locality::LocalityMatrixSystem*>(&matrixSystem); lmsPtr != nullptr) {
                return SymbolTableExporter{this->matlabEngine, *this->settings, *lmsPtr};
            } else if (auto imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&matrixSystem);
                       imsPtr != nullptr) {
                return SymbolTableExporter{this->matlabEngine, *this->settings, *imsPtr};
            }
            return SymbolTableExporter{this->matlabEngine, *this->settings, matrixSystem};
        }();

        // Output
        switch (input.output_mode) {
            case SymbolTableParams::OutputMode::AllSymbols:
                output[0] = exporter.export_table();
                break;
            case SymbolTableParams::OutputMode::FromId:
                output[0] = exporter.export_table(input.from_id);
                break;
            case SymbolTableParams::OutputMode::SearchBySequence:
                output[0] = find_and_return_symbol(input, exporter);
                break;
            case SymbolTableParams::OutputMode::SearchBySequenceArray:
                output[0] = find_and_return_symbol_array(input, exporter);
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output mode.");

        }

    }

    matlab::data::Array
    SymbolTable::find_and_return_symbol(const SymbolTableParams& input, const SymbolTableExporter &exporter) {
        matlab::data::ArrayFactory factory;

        const auto& system = exporter.system;
        const auto& context = system.Context();
        const auto& symbolTable = system.Symbols();

        // Try to find sequence
        const auto& seq = input.sequences.front();
        OperatorSequence trialSequence(sequence_storage_t(seq.begin(), seq.end()), context);
        auto symbolRow = symbolTable.where(trialSequence);

        // Return false if nothing found
        if (!symbolRow.found()) {
            return exporter.export_empty_row(true);
        }

        // Otherwise, export row
        return exporter.export_row(*symbolRow.symbol, symbolRow.is_conjugated, symbolRow.is_aliased);
    }

    matlab::data::Array
    SymbolTable::find_and_return_symbol_array(const SymbolTableParams& input, const SymbolTableExporter &exporter) {
        matlab::data::ArrayFactory factory;

        const auto& system = exporter.system;
        const auto& context = system.Context();
        const auto& symbolTable = system.Symbols();

        const size_t row_count = input.sequences.size();
        std::vector<SymbolLookupResult> results;

        for (size_t index = 0; index < row_count; ++ index) {
            const auto& seqRaw = input.sequences[index];
            OperatorSequence trialSequence(sequence_storage_t(seqRaw.begin(), seqRaw.end()), context);
            results.emplace_back(symbolTable.where(trialSequence));
        }
        return exporter.export_row_array(input.sequence_dimensions, std::span(results));
    }
}