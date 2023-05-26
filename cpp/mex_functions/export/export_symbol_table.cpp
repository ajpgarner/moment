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
        std::vector<std::string> column_names(const bool non_herm, const bool include_factors) {
            std::vector<std::string> table_fields{"symbol", "operators"};
            if (non_herm) {
                table_fields.emplace_back("conjugate");
                table_fields.emplace_back("hermitian");
            }
            table_fields.emplace_back("basis_re");
            if (non_herm) {
                table_fields.emplace_back("basis_im");
            }
            if (include_factors) {
                table_fields.emplace_back("fundamental");
                table_fields.emplace_back("factor_sequence");
                table_fields.emplace_back("factor_symbols");
                table_fields.emplace_back("factor_appearances");
            }
            return table_fields;
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


        void write_row(matlab::engine::MATLABEngine &engine, const EnvironmentalVariables &env,
                       matlab::data::ArrayFactory &factory,
                       const bool non_herm, const bool include_factors,
                       const Inflation::FactorTable *factorTablePtr, const Locality::LocalityContext *localityContextPtr,
                       const bool locality_format, matlab::data::StructArray &outputStruct, size_t write_index,
                       const Symbol &symbol) {

            outputStruct[write_index]["symbol"] = factory.createScalar<uint64_t>(static_cast<uint64_t>(symbol.Id()));
            if (locality_format) {
                outputStruct[write_index]["operators"] =  factory.createScalar(
                        localityContextPtr->format_sequence(*env.get_locality_formatter(),symbol.sequence()));
            } else {
                outputStruct[write_index]["operators"] = factory.createScalar(symbol.formatted_sequence());
            }

            // +1 is from MATLAB indexing
            outputStruct[write_index]["basis_re"] = factory.createScalar<uint64_t>(symbol.basis_key().first + 1);

            if (non_herm) {
                if (locality_format) {
                    outputStruct[write_index]["conjugate"] = factory.createScalar(
                            localityContextPtr->format_sequence(*env.get_locality_formatter(), symbol.sequence_conj()));
                } else {
                    outputStruct[write_index]["conjugate"] = factory.createScalar(symbol.formatted_sequence_conj());
                }

                outputStruct[write_index]["hermitian"] = factory.createScalar<bool>(symbol.is_hermitian());
                // +1 is from MATLAB indexing
                outputStruct[write_index]["basis_im"] = factory.createScalar<uint64_t>(symbol.basis_key().second + 1);
            }
            if (include_factors) {
                const auto& entry = (*factorTablePtr)[symbol.Id()];

                outputStruct[write_index]["fundamental"] = factory.createScalar<bool>(entry.fundamental());
                outputStruct[write_index]["factor_sequence"] = factory.createScalar(entry.sequence_string());
                outputStruct[write_index]["factor_symbols"] = make_factor_symbol_array(factory, entry);
                outputStruct[write_index]["factor_appearances"] = factory.createScalar(entry.appearances);
            }
        }
    }

    matlab::data::StructArray export_symbol_table_row(matlab::engine::MATLABEngine& engine,
                                                      const EnvironmentalVariables& env,
                                                      const MatrixSystem& system, const Symbol& symbol) {

        matlab::data::ArrayFactory factory;
        const auto& context = system.Context();

        // Ascertain system properties
        const bool non_herm = context.can_be_nonhermitian();
        const auto* inflationContextPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
        const bool include_factors = (nullptr != inflationContextPtr);
        const Inflation::FactorTable* factorTablePtr = nullptr;
        if (include_factors) {
            factorTablePtr = &inflationContextPtr->Factors();
        }

        const auto* localityContextPtr = dynamic_cast<const Locality::LocalityContext*>(&(system.Context()));
        const bool locality_format = (nullptr != localityContextPtr);

        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, 1},
                                                      column_names(non_herm, include_factors));

        // Write single row
        write_row(engine, env, factory, non_herm, include_factors, factorTablePtr,
                  localityContextPtr, locality_format, outputStruct, 0, symbol);

        return outputStruct;
    }


    matlab::data::StructArray export_symbol_table_struct(matlab::engine::MATLABEngine& engine,
                                                         const EnvironmentalVariables& env,
                                                         const MatrixSystem& system,
                                                         const size_t from_symbol) {
        matlab::data::ArrayFactory factory;

        const auto& table = system.Symbols();
        const auto& context = system.Context();


        // Advance to first new symbol (if necessary)
        auto symbolIter = table.begin();
        if (from_symbol > 0) {
            if (from_symbol < table.size()) {
                symbolIter += static_cast<ptrdiff_t>(from_symbol);
            } else {
                symbolIter = table.end();
            }
        }

        // Number of symbols to be output
        const size_t num_elems = (from_symbol < table.size()) ? (table.size() - from_symbol) : 0;

        // Ascertain table field names
        const bool non_herm = context.can_be_nonhermitian();
        const auto* inflationContextPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
        const bool include_factors = (nullptr != inflationContextPtr);
        const Inflation::FactorTable* factorTablePtr = nullptr;
        if (include_factors) {
            factorTablePtr = &inflationContextPtr->Factors();
        }

        const auto* localityContextPtr = dynamic_cast<const Locality::LocalityContext*>(&(system.Context()));
        const bool locality_format = (nullptr != localityContextPtr);

        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, num_elems},
                                                      column_names(non_herm, include_factors));

        // Early exit, if empty
        if (0 == num_elems) {
            return outputStruct;
        }

        // Copy rest of table:
        size_t write_index = 0;
        while (symbolIter != table.end()) {
            const auto& symbol = *symbolIter;
            if (write_index >= num_elems) {
                throw_error(engine, errors::internal_error,
                            "Unexpectedly many sequences in export_symbol_table_struct.");
            }

            write_row(engine, env, factory, non_herm, include_factors, factorTablePtr,
                      localityContextPtr, locality_format, outputStruct, write_index, symbol);

            ++write_index;
            ++symbolIter;
        }

        return outputStruct;
    }
}