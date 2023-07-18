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
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_operator_formatter.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/factor_table.h"

#include "utilities/reporting.h"

namespace Moment::mex {
    namespace {
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
            : ExporterWithFactory{engine}, env{env}, system{system},
              symbols{system.Symbols()}, context{system.Context()},
              include_factors{false}, locality_format{false}, can_have_aliases{false} {
     }


    SymbolTableExporter::SymbolTableExporter(matlab::engine::MATLABEngine &engine, const EnvironmentalVariables &env,
                                             const Locality::LocalityMatrixSystem &lms)
         : ExporterWithFactory{engine}, env{env}, system{lms}, symbols{system.Symbols()}, context{system.Context()},
            include_factors{false}, locality_format{true}, can_have_aliases{false} {
        this->localityContextPtr = &lms.localityContext;
    }

    SymbolTableExporter::SymbolTableExporter(matlab::engine::MATLABEngine &engine, const EnvironmentalVariables &env,
                                             const Inflation::InflationMatrixSystem &ims)
            : ExporterWithFactory{engine}, env{env}, system{ims}, symbols{system.Symbols()}, context{system.Context()},
              include_factors{true}, locality_format{false}, can_have_aliases{true} {
        this->factorTablePtr = &ims.Factors();
    }

    std::vector<std::string>
    SymbolTableExporter::column_names(const bool look_up_mode) const {
        std::vector<std::string> table_fields{"symbol", "operators"};

        if (look_up_mode) {
            table_fields.emplace_back("conjugated");
            if (this->can_have_aliases) {
                table_fields.emplace_back("is_alias");
            }
        }

        table_fields.emplace_back("conjugate");
        table_fields.emplace_back("hermitian");

        table_fields.emplace_back("basis_re");
        table_fields.emplace_back("basis_im");

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
    SymbolTableExporter::export_row(const Symbol& symbol,
                                    std::optional<bool> conjugated,
                                    std::optional<bool> is_alias) const {
        const bool lookup_mode = conjugated.has_value();

        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, 1},
                                                      column_names(lookup_mode));

        // Write single row
        auto write_iter = outputStruct.begin();
        this->do_row_write(write_iter, symbol, conjugated, this->can_have_aliases ? is_alias : std::nullopt);

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

            this->do_row_write(write_iter, symbol, std::nullopt, std::nullopt);

            ++write_iter;
            ++symbolIter;
        }
        return outputStruct;
    }

    matlab::data::StructArray
    SymbolTableExporter::export_row_array(std::span<const size_t> shape,
                                          std::span<const SymbolLookupResult> symbol_info) const {

        const size_t expected_length = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies());
        if (expected_length != symbol_info.size()) {
            throw_error(engine, errors::internal_error,
                        "Number of symbol IDs requested does not match the desired output shape.");
        }

        // Construct structure array
        matlab::data::ArrayDimensions array_dims{shape.begin(), shape.end()};
        auto outputStruct = factory.createStructArray(std::move(array_dims), column_names(true));

        auto write_iter = outputStruct.begin();
        for (size_t index = 0; index < expected_length; ++index) {
            const auto& symbol = symbol_info[index];
            if (!symbol.found()) {
                this->do_missing_row_write(write_iter, true);
            } else {
                this->do_row_write(write_iter, *symbol, symbol.is_conjugated, symbol.is_aliased);
            }
            ++write_iter;
        }

        return outputStruct;
    }

    void SymbolTableExporter::do_row_write(matlab::data::StructArray::iterator& write_iter,
                                           const Symbol &symbol, const std::optional<bool> conjugated,
                                           const std::optional<bool> is_aliased) const {

        (*write_iter)["symbol"] = factory.createScalar<int64_t>(static_cast<int64_t>(symbol.Id()));
        if (this->locality_format) {
            (*write_iter)["operators"] = factory.createScalar(
                    localityContextPtr->format_sequence(*env.get_locality_formatter(), symbol.sequence()));
        } else {
            (*write_iter)["operators"] = factory.createScalar(symbol.formatted_sequence());
        }

        if (conjugated.has_value()) {
            (*write_iter)["conjugated"] = factory.createScalar<bool>(conjugated.value());
        }
        if (this->can_have_aliases && is_aliased.has_value()) {
            (*write_iter)["is_alias"] = factory.createScalar<bool>(is_aliased.value());
        }

        // +1 is from MATLAB indexing
        (*write_iter)["basis_re"] = factory.createScalar<uint64_t>(symbol.basis_key().first + 1);


        if (this->locality_format) {
            (*write_iter)["conjugate"] = factory.createScalar(
                    this->localityContextPtr->format_sequence(*env.get_locality_formatter(), symbol.sequence_conj()));
        } else {
            (*write_iter)["conjugate"] = factory.createScalar(symbol.formatted_sequence_conj());
        }

        (*write_iter)["hermitian"] = factory.createScalar<bool>(symbol.is_hermitian());
        // +1 is from MATLAB indexing
        (*write_iter)["basis_im"] = factory.createScalar<uint64_t>(symbol.basis_key().second + 1);

        if (this->include_factors) {
            const auto& entry = (*factorTablePtr)[symbol.Id()];
            (*write_iter)["fundamental"] = factory.createScalar<bool>(entry.fundamental());
            (*write_iter)["factor_sequence"] = factory.createScalar(entry.sequence_string());
            (*write_iter)["factor_symbols"] = make_factor_symbol_array(factory, entry);
            (*write_iter)["factor_appearances"] = factory.createScalar(entry.appearances);
        }
    }


    void SymbolTableExporter::do_missing_row_write(matlab::data::StructArray::iterator& write_iter,
                                                   const bool lookup_mode) const {

        (*write_iter)["symbol"] = factory.createScalar<int64_t>(-1);
        (*write_iter)["operators"] = factory.createScalar("");

        if (lookup_mode) {
            (*write_iter)["conjugated"] = factory.createScalar<bool>(false);
            if (this->can_have_aliases) {
                (*write_iter)["is_alias"] = factory.createScalar<bool>(false);
            }
        }

        // +1 is from MATLAB indexing
        (*write_iter)["basis_re"] = factory.createScalar<uint64_t>(0);

        (*write_iter)["conjugate"] = factory.createScalar("");
        (*write_iter)["hermitian"] = factory.createScalar<bool>(false);
        // +1 is from MATLAB indexing
        (*write_iter)["basis_im"] = factory.createScalar<uint64_t>(0);

        if (this->include_factors) {
            (*write_iter)["fundamental"] = factory.createScalar<bool>(false);
            (*write_iter)["factor_sequence"] = factory.createScalar("");
            (*write_iter)["factor_symbols"] = factory.createArray<uint64_t>({1, 0});
            (*write_iter)["factor_appearances"] = factory.createScalar(0);
        }
    }
}