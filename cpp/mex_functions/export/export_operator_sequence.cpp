/**
 * export_operator_sequence.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_operator_sequence.h"
#include "utilities/write_as_array.h"

#include "scenarios/operator_sequence.h"
#include "matrix/operator_sequence_generator.h"

namespace Moment::mex {

    matlab::data::TypedArray<uint64_t>
    export_operator_sequence(matlab::data::ArrayFactory &factory,
                             const OperatorSequence &sequence, const bool matlab_indexing) {

        auto output = write_as_array<uint64_t>(factory, sequence.begin(), sequence.end(), true);
        // Apply matlab indexing
        if (matlab_indexing) {
            for (auto& e : output) {
                e += 1;
            }
        }
        return output;
    }

    matlab::data::TypedArray<matlab::data::Array>
    export_all_operator_sequences(matlab::data::ArrayFactory &factory,
                                  const OperatorSequenceGenerator &osg,
                                  const bool matlab_indexing) {

        auto output = factory.createCellArray(matlab::data::ArrayDimensions{osg.size(), 1});

        auto write_iter = output.begin();
        for (const auto& seq : osg) {
            *write_iter = export_operator_sequence(factory, seq, matlab_indexing);
            ++write_iter;
        }
        assert(write_iter == output.end());

        return output;
    }
}
