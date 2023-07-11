/**
 * export_symbol_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_symbol_table.h"

#include "error_codes.h"
#include "environmental_variables.h"

#include "symbolic/symbol_table.h"

#include "scenarios/context.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_operator_formatter.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/factor_table.h"

#include "utilities/reporting.h"

namespace Moment::mex {
    namespace {

        inline bool query_can_be_nonhermitian(const MatrixSystem &system) {
            const auto& context = system.Context();
            return context.can_be_nonhermitian();
        }

        inline bool query_can_have_factors(const MatrixSystem &system) {
            const auto* inflationContextPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
            return (nullptr != inflationContextPtr);
        }

        inline bool query_locality_format(const MatrixSystem &system) {
            const auto *localityContextPtr = dynamic_cast<const Locality::LocalityContext *>(&(system.Context()));
            return (nullptr != localityContextPtr);
        }


        matlab::data::TypedArray<uint64_t>
        make_factor_symbol_array(matlab::data::ArrayFactory &factory, const Inflation::FactorTable::FactorEntry &entry) {
            auto factor_symbols = factory.createArray<uint64_t>({1, entry.canonical.symbols.size()});
            auto fsWriteIter = factor_symbols.begin();
            for (const auto symbolId : entry.canonical.symbols) {
                *fsWriteIter = symbolId;
                ++fsWriteIter;
            }
            return factor_symbols;
        }



    }


    SymbolTableExporter::SymbolTableExporter(matlab::engine::MATLABEngine &engine, const EnvironmentalVariables &env,
                                             const MatrixSystem &system)
            : ExporterWithFactory{engine}, env{env}, system{system}, symbols{system.Symbols()}, context{system.Context()},
              can_be_nonhermitian{query_can_be_nonhermitian(system)},
              include_factors{query_can_have_factors(system)},
              locality_format{query_locality_format(system)} {
        if (this->locality_format) {
            this->localityContextPtr = dynamic_cast<const Locality::LocalityContext *>(&(system.Context()));
            assert(this->localityContextPtr != nullptr);
        }

        if (this->include_factors) {
            const auto* inflationContextPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
            assert(inflationContextPtr != nullptr);
            this->factorTablePtr = &inflationContextPtr->Factors();
        }
    }


    std::vector<std::string>
    SymbolTableExporter::column_names(const bool include_conj) const {
        std::vector<std::string> table_fields{"symbol", "operators"};

        if (include_conj) {
            table_fields.emplace_back("conjugated");
        }

        if (this->can_be_nonhermitian) {
            table_fields.emplace_back("conjugate");
            table_fields.emplace_back("hermitian");
        }

        table_fields.emplace_back("basis_re");
        if (this->can_be_nonhermitian) {
            table_fields.emplace_back("basis_im");
        }

        if (this->include_factors) {
            table_fields.emplace_back("fundamental");
            table_fields.emplace_back("factor_sequence");
            table_fields.emplace_back("factor_symbols");
            table_fields.emplace_back("factor_appearances");
        }
        return table_fields;
    }

    matlab::data::StructArray SymbolTableExporter::export_empty_row(bool include_conj) const {
        // Construct structure array
        return factory.createStructArray(matlab::data::ArrayDimensions{1, 0}, this->column_names(include_conj));
    }


    matlab::data::StructArray
    SymbolTableExporter::export_row(const Symbol& symbol, std::optional<bool> conjugated) const {
        const auto& context = system.Context();

        const bool include_conj = conjugated.has_value();

        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, 1}, column_names(include_conj));

        // Write single row
        auto write_iter = outputStruct.begin();
        this->do_row_write(write_iter, symbol, conjugated);

        return outputStruct;
    }

    matlab::data::StructArray SymbolTableExporter::export_table(const size_t from_symbol) const {
        // Advance to first new symbol (if necessary)
        auto symbolIter = this->symbols.begin();
        if (from_symbol > 0) {
            if (from_symbol < this->symbols.size()) {
                symbolIter += static_cast<ptrdiff_t>(from_symbol);
            } else {
                symbolIter = this->symbols.end();
            }
        }

        // Number of symbols to be output
        const size_t num_elements = (from_symbol < this->symbols.size()) ? (this->symbols.size() - from_symbol) : 0;

        // Early exit, if empty
        if (0 == num_elements) {
            return this->export_empty_row(false);
        }


        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, num_elements},
                                                      column_names(false));

        // Copy rest of table:
        auto write_iter = outputStruct.begin();
        while (symbolIter != this->symbols.end()) {
            const auto& symbol = *symbolIter;
            if (write_iter == outputStruct.end()) {
                throw_error(engine, errors::internal_error,
                            "Unexpectedly many sequences in export_symbol_table_struct.");
            }

            this->do_row_write(write_iter, symbol, std::nullopt);

            ++write_iter;
            ++symbolIter;
        }
        return outputStruct;
    }

    matlab::data::StructArray SymbolTableExporter::export_row_array(std::span<const size_t> shape,
                                                                    std::span<const symbol_name_t> symbol_ids,
                                                                    std::span<const uint8_t> conj_status) const {
        const bool include_conjugates = !conj_status.empty();
        if (include_conjugates && (conj_status.size() != symbol_ids.size())) {
            throw_error(engine, errors::internal_error,
                        "Conjugate status array size does not match symbol ID array.");
        }
        const size_t expected_length = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies());
        if (expected_length != symbol_ids.size()) {
            throw_error(engine, errors::internal_error,
                        "Number of symbol IDs requested does not match the desired output shape.");
        }

        // Construct structure array
        matlab::data::ArrayDimensions array_dims{shape.begin(), shape.end()};
        auto outputStruct = factory.createStructArray(std::move(array_dims), column_names(include_conjugates));

        // Function: Is conjugated or don't care.
        auto is_conjugated = [&](size_t index) -> std::optional<bool> {
            if (include_conjugates) {
                return conj_status[index];
            }
            return std::nullopt;
        };

        auto write_iter = outputStruct.begin();
        for (size_t index = 0; index < expected_length; ++index) {
            const auto symbol_id = symbol_ids[index];
            if ((symbol_id < 0) || (symbol_id >= this->symbols.size())) {
                this->do_missing_row_write(write_iter, include_conjugates);
            } else {
                const auto &symbolInfo = this->symbols[symbol_id];
                const auto conjugated = is_conjugated(index);
                this->do_row_write(write_iter, symbolInfo, conjugated);
            }
            ++write_iter;
        }

        return outputStruct;
    }

    void SymbolTableExporter::do_row_write(matlab::data::StructArray::iterator& write_iter,
                                           const Symbol &symbol, std::optional<bool> conjugated) const {

        (*write_iter)["symbol"] = factory.createScalar<int64_t>(static_cast<int64_t>(symbol.Id()));
        if (this->locality_format) {
            (*write_iter)["operators"] =  factory.createScalar(
                    localityContextPtr->format_sequence(*env.get_locality_formatter(), symbol.sequence()));
        } else {
            (*write_iter)["operators"] = factory.createScalar(symbol.formatted_sequence());
        }

        if (conjugated.has_value()) {
            (*write_iter)["conjugated"] = factory.createScalar(conjugated.value());
        }

        // +1 is from MATLAB indexing
        (*write_iter)["basis_re"] = factory.createScalar<uint64_t>(symbol.basis_key().first + 1);

        if (this->can_be_nonhermitian) {
            if (this->locality_format) {
                (*write_iter)["conjugate"] = factory.createScalar(
                        this->localityContextPtr->format_sequence(*env.get_locality_formatter(), symbol.sequence_conj()));
            } else {
                (*write_iter)["conjugate"] = factory.createScalar(symbol.formatted_sequence_conj());
            }

            (*write_iter)["hermitian"] = factory.createScalar<bool>(symbol.is_hermitian());
            // +1 is from MATLAB indexing
            (*write_iter)["basis_im"] = factory.createScalar<uint64_t>(symbol.basis_key().second + 1);
        }
        if (this->include_factors) {
            const auto& entry = (*factorTablePtr)[symbol.Id()];

            (*write_iter)["fundamental"] = factory.createScalar<bool>(entry.fundamental());
            (*write_iter)["factor_sequence"] = factory.createScalar(entry.sequence_string());
            (*write_iter)["factor_symbols"] = make_factor_symbol_array(factory, entry);
            (*write_iter)["factor_appearances"] = factory.createScalar(entry.appearances);
        }
    }


    void SymbolTableExporter::do_missing_row_write(matlab::data::StructArray::iterator& write_iter,
                                                   const bool include_conj) const {

        (*write_iter)["symbol"] = factory.createScalar<int64_t>(-1);
        (*write_iter)["operators"] = factory.createScalar("");

        if (include_conj) {
            (*write_iter)["conjugated"] = factory.createScalar<bool>(false);
        }

        // +1 is from MATLAB indexing
        (*write_iter)["basis_re"] = factory.createScalar<uint64_t>(0);

        if (this->can_be_nonhermitian) {
            (*write_iter)["conjugate"] = factory.createScalar("");
            (*write_iter)["hermitian"] = factory.createScalar<bool>(false);
            // +1 is from MATLAB indexing
            (*write_iter)["basis_im"] = factory.createScalar<uint64_t>(0);
        }

        if (this->include_factors) {
            (*write_iter)["fundamental"] = factory.createScalar<bool>(false);
            (*write_iter)["factor_sequence"] = factory.createScalar("");
            (*write_iter)["factor_symbols"] = factory.createArray<uint64_t>({1, 0});
            (*write_iter)["factor_appearances"] = factory.createScalar(0);
        }
    }
}