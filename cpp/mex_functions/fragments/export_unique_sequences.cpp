/**
 * export_unique_sequences.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_unique_sequences.h"

#include "error_codes.h"
#include "operators/context.h"
#include "operators/matrix/moment_matrix.h"
#include "utilities/reporting.h"

namespace NPATK::mex {
    matlab::data::StructArray export_unique_sequence_struct(matlab::engine::MATLABEngine& engine,
                                                            const MomentMatrix& mm) {
        const Context& context = mm.context;
        const auto& usr = mm.Symbols; // .UniqueSequences;
        matlab::data::ArrayFactory factory;
        const size_t num_elems = usr.size();
        matlab::data::ArrayDimensions array_dims{1, num_elems};

        auto outputStruct = factory.createStructArray(std::move(array_dims),
                                                      {"symbol", "operators", "conjugate", "real",
                                                       "basis_re", "basis_im"});
        size_t write_index = 0;


        const auto& basisMap = mm.SMP().BasisMap();
        auto basisMapIter = basisMap.cbegin();

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
            if (0 == write_index ) {
                outputStruct[write_index]["basis_re"] = factory.createScalar<uint64_t>(0);
                outputStruct[write_index]["basis_im"] = factory.createScalar<uint64_t>(0);
            } else {
                assert(basisMapIter->first == symbol.Id());
                outputStruct[write_index]["basis_re"] = factory.createScalar<uint64_t>(basisMapIter->second.first + 1);
                outputStruct[write_index]["basis_im"] = factory.createScalar<uint64_t>(basisMapIter->second.second + 1);
                ++basisMapIter;
            }

            ++write_index;
        }

        return outputStruct;
    }
}