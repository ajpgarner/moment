/**
 * moment_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "moment_matrix.h"

namespace NPATK::mex {
    namespace classes {

        MomentMatrix::MomentMatrix(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput)
                : MATLABClass(engine, "MomentMatrix",
                              MATLABClass::FieldTypeMap{
                                      {"RefId", matlab::data::ArrayType::UINT64},
                                      {"Level",  matlab::data::ArrayType::UINT64},
                              }, std::move(rawInput)) {
            this->reference_key = this->property_scalar<uint64_t>("RefId");
            this->level = this->property_scalar<uint64_t>("Level");
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