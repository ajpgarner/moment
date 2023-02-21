/**
 * export_operator_sequence.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_operator_sequence.h"
#include "utilities/write_as_array.h"

#include "scenarios/operator_sequence.h"

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
}
