/**
 * export_symbol_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_symbol_table.h"

#include "error_codes.h"
#include "operators/context.h"
#include "operators/matrix/symbol_table.h"
#include "utilities/reporting.h"

namespace NPATK::mex {
    matlab::data::StructArray export_symbol_table_struct(matlab::engine::MATLABEngine& engine,
                                                            const Context& context,
                                                            const SymbolTable& table,
                                                            const size_t from_symbol) {
        matlab::data::ArrayFactory factory;

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

        // Construct structure array
        auto outputStruct = factory.createStructArray(matlab::data::ArrayDimensions{1, num_elems},
                                                      {"symbol", "operators", "conjugate", "hermitian",
                                                       "basis_re", "basis_im"});

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
            outputStruct[write_index]["symbol"] = factory.createScalar<uint64_t>(static_cast<uint64_t>(symbol.Id()));
            outputStruct[write_index]["operators"] = factory.createScalar(
                    context.format_sequence(symbol.sequence()));
            outputStruct[write_index]["conjugate"] = factory.createScalar(
                    context.format_sequence(symbol.sequence_conj()));
            outputStruct[write_index]["hermitian"] = factory.createScalar<bool>(symbol.is_hermitian());

            // +1 is from MATLAB indexing
            outputStruct[write_index]["basis_re"] = factory.createScalar<uint64_t>(symbol.basis_key().first + 1);
            outputStruct[write_index]["basis_im"] = factory.createScalar<uint64_t>(symbol.basis_key().second + 1);

            ++write_index;
            ++symbolIter;
        }

        return outputStruct;
    }
}