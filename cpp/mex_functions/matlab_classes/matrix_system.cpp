/**
 * matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_system.h"

namespace NPATK::mex::classes {
    MatrixSystem::MatrixSystem(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput)
            : MATLABClass(engine, "MatrixSystem",
                          MATLABClass::FieldTypeMap{
                                  {"RefId", matlab::data::ArrayType::UINT64}
                          }, std::move(rawInput)) {
        this->reference_key = this->property_scalar<uint64_t>("RefId");
    }
}