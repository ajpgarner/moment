/**
 * derived_matrix_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "derived_matrix_indices.h"
#include "derived_matrix_system.h"

#include "matrix/symbolic_matrix.h"

#include <sstream>

namespace Moment::Derived {


    std::string DerivedMatrixIndex::to_string(DerivedMatrixSystem&) const {
        return this->to_string();
    }

    std::string DerivedMatrixIndex::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::ostream& operator<<(std::ostream& os, DerivedMatrixIndex dmi) {
        os << "Derived Matrix: Source Index " << dmi.SourceIndex;
        return os;
    }

    DerivedMatrixFactory::DerivedMatrixFactory(MatrixSystem& system)
        : system{dynamic_cast<DerivedMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, SymbolicMatrix&>
    DerivedMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock, Index src_offset,
                                     Multithreading::MultiThreadPolicy mt_policy) {

        auto derived_matrix = this->system.create_derived_matrix(lock, src_offset, mt_policy);
        auto& matrix_ref = *derived_matrix;
        auto matrix_offset = this->system.push_back(lock, std::move(derived_matrix));
        return {matrix_offset, matrix_ref};
    }

    void DerivedMatrixFactory::notify(const MaintainsMutex::WriteLock& lock, Index src_offset,
                                      ptrdiff_t target_offset, SymbolicMatrix& target_matrix) {
        this->system.on_new_derived_matrix(lock, src_offset, target_offset, target_matrix);
    }

}