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

    matlab::data::TypedArray<uint64_t>
    export_operator_sequence(matlab::data::ArrayFactory& factory, const OperatorSequence& sequence, bool offset = true);

}
