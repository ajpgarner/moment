/**
 * read_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace matlab::data {
    class Array;
}
namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class MatrixSystem;
    namespace mex {
        class StorageManager;

        class MatrixSystemId {
        private:
            matlab::engine::MATLABEngine& matlabEngine;

        public:
            /** The label of the parameter */
            const std::string param_name;

        protected:
            uint64_t key;

        public:
            MatrixSystemId(matlab::engine::MATLABEngine& engine, std::string param_name_in = "Matrix system reference") noexcept
                 : matlabEngine{engine}, param_name{std::move(param_name_in)} { }

            /**
             * Parse a MATLAB array into a key for the matrix system.
             * @param input_array MATLAB array that can be interpreted as an unsigned integer.
             * @return The matrix system key.
             * @throws matlab exceptions if number cannot be read, or does not match signature of matrix system.
             */
            uint64_t parse_input(const matlab::data::Array& input_array);

            std::shared_ptr<MatrixSystem> operator()(StorageManager& manager) const;

            friend std::ostream& operator<<(std::ostream& os, const MatrixSystemId& msi);
        };
    }
}