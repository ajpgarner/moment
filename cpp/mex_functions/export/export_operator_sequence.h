/**
 * export_operator_sequence.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "MatlabDataArray.hpp"

namespace Moment {
    class OperatorSequence;
}

namespace Moment::mex {
    /**
     * Export operator sequence as a row-vector of unsigned 64-bit integers.
     * @param factory MATLAB array factory.
     * @param sequence The operator sequence to export.
     * @param offset If true, add +1 to each operator number (to match with MATLAB indexing).
     * @return Newly constructed MATLAB array representing sequence.
     */
    matlab::data::TypedArray<uint64_t>
    export_operator_sequence(matlab::data::ArrayFactory& factory, const OperatorSequence& sequence, bool offset = true);
}
