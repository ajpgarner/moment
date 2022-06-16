/**
 * export_unique_sequences.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "operators/moment_matrix.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {

    matlab::data::StructArray export_unique_sequence_struct(matlab::engine::MATLABEngine& engine,
                                                            const Context& context,
                                                            const MomentMatrix::UniqueSequenceRange& usr);

}
