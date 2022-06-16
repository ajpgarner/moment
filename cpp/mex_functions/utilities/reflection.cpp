/**
 * reflection.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "reflection.h"

#include <sstream>

namespace NPATK::mex {

    std::string to_string(const matlab::data::ArrayType &arrayType) {
        switch (arrayType) {
            case matlab::data::ArrayType::LOGICAL:
                return "Logical";
            case matlab::data::ArrayType::CHAR:
                return "Char";
            case matlab::data::ArrayType::MATLAB_STRING:
                return "MATLAB String";
            case matlab::data::ArrayType::DOUBLE:
                return "Double";
            case matlab::data::ArrayType::SINGLE:
                return "Single";
            case matlab::data::ArrayType::INT8:
                return "Int8";
            case matlab::data::ArrayType::UINT8:
                return "Unsigned Int8";
            case matlab::data::ArrayType::INT16:
                return "Int16";
            case matlab::data::ArrayType::UINT16:
                return "Unsigned Int16";
            case matlab::data::ArrayType::INT32:
                return "Int32";
            case matlab::data::ArrayType::UINT32:
                return "Unsigned Int32";
            case matlab::data::ArrayType::INT64:
                return "Int64";
            case matlab::data::ArrayType::UINT64:
                return "Unsigned Int64";
            case matlab::data::ArrayType::COMPLEX_DOUBLE:
                return "Complex Double";
            case matlab::data::ArrayType::COMPLEX_SINGLE:
                return "Complex Single";
            case matlab::data::ArrayType::COMPLEX_INT8:
                return "Complex Int8";
            case matlab::data::ArrayType::COMPLEX_UINT8:
                return "Complex Unsigned Int8";
            case matlab::data::ArrayType::COMPLEX_INT16:
                return "Complex Int16";
            case matlab::data::ArrayType::COMPLEX_UINT16:
                return "Complex Unsigned Int16";
            case matlab::data::ArrayType::COMPLEX_INT32:
                return "Complex Int32";
            case matlab::data::ArrayType::COMPLEX_UINT32:
                return "Complex Unsigned Int32";
            case matlab::data::ArrayType::COMPLEX_INT64:
                return "Complex Int64";
            case matlab::data::ArrayType::COMPLEX_UINT64:
                return "Complex Unsigned Int64";
            case matlab::data::ArrayType::CELL:
                return "Cell";
            case matlab::data::ArrayType::STRUCT:
                return "Struct";
            case matlab::data::ArrayType::OBJECT:
                return "Object";
            case matlab::data::ArrayType::VALUE_OBJECT:
                return "Value Object";
            case matlab::data::ArrayType::HANDLE_OBJECT_REF:
                return "Handle Object Ref";
            case matlab::data::ArrayType::ENUM:
                return "Enum";
            case matlab::data::ArrayType::SPARSE_LOGICAL:
                return "Sparse Logical Array";
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                return "Sparse Double Array";
            case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE:
                return "Sparse Complex Double Array";
            default:
            case matlab::data::ArrayType::UNKNOWN:
                return "Unknown";
        }
    }

    std::string summary_string(const matlab::data::Array &array) {
        std::stringstream ss;
        ss << type_as_string(array) << ": ";

        if (array.isEmpty()) {
            ss << "Empty";
        } else if (array.getNumberOfElements() == 1) {
            ss << "Scalar";
        } else {
            ss << "Array ";
            auto dims = array.getDimensions();
            bool one_dim = false;
            for (const auto &dim: dims) {
                if (one_dim) {
                    ss << " x ";
                }
                ss << dim;
                one_dim = true;
            }
        }
        return ss.str();
    }

}
