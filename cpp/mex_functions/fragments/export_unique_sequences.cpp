/**
 * export_unique_sequences.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_unique_sequences.h"

#include "operators/context.h"
#include "utilities/reporting.h"
#include "error_codes.h"

namespace NPATK::mex {
    matlab::data::StructArray export_unique_sequence_struct(matlab::engine::MATLABEngine& engine,
                                                            const Context& context,
                                                            const MomentMatrix::UniqueSequenceRange& usr) {
        matlab::data::ArrayFactory factory;
        const size_t num_elems = usr.size();
        matlab::data::ArrayDimensions array_dims{1, num_elems};

        auto outputStruct = factory.createStructArray(std::move(array_dims),
                                                      {"symbol", "operators", "conjugate", "real"});
        size_t write_index = 0;
        for (const auto& symbol : usr) {
            if (write_index >= num_elems) {
                throw_error(engine, errors::internal_error,
                            "Unexpectedly many sequences in export_unique_sequence_struct.");
            }
            outputStruct[write_index]["symbol"] = factory.createScalar<uint64_t>(static_cast<uint64_t>(symbol.Id()));
            outputStruct[write_index]["operators"] = factory.createScalar(
                    context.format_sequence(symbol.sequence()));
            outputStruct[write_index]["conjugate"] = factory.createScalar(
                    context.format_sequence(symbol.sequence_conj()));
            outputStruct[write_index]["real"] = factory.createScalar<bool>(symbol.is_hermitian());

            ++write_index;
        }

        return outputStruct;
    }
}