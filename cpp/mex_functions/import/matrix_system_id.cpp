/**
 * read_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_system_id.h"

#include "errors.h"
#include "storage_manager.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include "MatlabDataArray.hpp"

#include <ostream>
#include <sstream>

namespace Moment::mex {

    uint64_t MatrixSystemId::parse_input(const matlab::data::Array& input_array) {
        this->key = read_positive_integer<uint64_t>(this->matlabEngine, this->param_name, input_array, 0);

        // Pre-check key matches expected prefix:
        if (!PersistentStorageBase::check_signature(StorageManager::matrix_system_signature, this->key)) {
            std::stringstream errSS;
            errSS << this->param_name << " was not the key to a valid matrix system.";
            throw StorageManagerError{errSS.str()};
        }

        return this->key;
    }

    std::shared_ptr<MatrixSystem> MatrixSystemId::operator()(StorageManager& manager) const {
        try {
            return manager.MatrixSystems.get(this->key);
        } catch (Moment::errors::bad_signature_error& bse) {
            // Report error to MATLAB if matrix system does not exist
            std::stringstream errSS;
            errSS << this->param_name << " was not the key to a valid matrix system.";
            throw StorageManagerError{errSS.str()};
        } catch (Moment::errors::persistent_object_error& poe) {
            // Report error to MATLAB if matrix system does not exist
            std::stringstream errSS;
            errSS << this->param_name << " was not the key to a valid matrix system: " << poe.what();
            throw StorageManagerError{errSS.str()};
        }
    }

    std::ostream& operator<<(std::ostream& os, const MatrixSystemId& msi) {
        os << msi.key;
        return os;
    }
}