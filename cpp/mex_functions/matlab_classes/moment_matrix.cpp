/**
 * moment_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "moment_matrix.h"
#include "matrix_system.h"

namespace NPATK::mex {
    namespace classes {

        MomentMatrix::MomentMatrix(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput)
                : MATLABClass(engine, "MomentMatrix",
                              MATLABClass::FieldTypeMap{
                                      {"Level",  matlab::data::ArrayType::UINT64},
                                      {"MatrixSystem", matlab::data::ArrayType::HANDLE_OBJECT_REF}
                              }, std::move(rawInput)) {

            // Extract moment matrix depth
            this->level = this->property_scalar<uint64_t>("Level");

            // Read handle to matrix system, and extract key.
            auto matSys = this->property("MatrixSystem");
            if (matSys.getNumberOfElements() != 1) {
                throw errors::bad_class_exception{this->className, "Only one MatrixSystem handle should be specified."};
            }
            MatrixSystem matrixSystem{engine, matSys};
            this->reference_key = matrixSystem.Key();
        }
    }



    std::pair<std::unique_ptr<classes::MomentMatrix>, std::optional<std::string>>
    read_as_moment_matrix(matlab::engine::MATLABEngine &engine,
                    matlab::data::Array raw_data) {

        // Check just one
        if (raw_data.getNumberOfElements() != 1) {
            return {nullptr, std::string("Only one MomentMatrix object should be supplied.")};
        }

        // Check object is an instance of 'MomentMatrix'
        std::unique_ptr<classes::MomentMatrix> ptrMM;
        try {
            ptrMM = std::make_unique<classes::MomentMatrix>(engine, std::move(raw_data));
        } catch (const errors::bad_class_exception &bce) {
            return {nullptr, std::string{bce.what()}};
        }

        return std::make_pair(std::move(ptrMM), std::nullopt);
    }
}