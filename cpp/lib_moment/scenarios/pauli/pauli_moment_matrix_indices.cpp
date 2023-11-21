/**
 * pauli_moment_matrix_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_moment_matrix_indices.h"

#include "pauli_matrix_system.h"
#include "matrix/monomial_matrix.h"

namespace Moment::Pauli {
    PauliMomentMatrixFactory::PauliMomentMatrixFactory(MatrixSystem &system)
            : PauliMomentMatrixFactory{dynamic_cast<PauliMatrixSystem&>(system)} { }


    std::pair<ptrdiff_t, MonomialMatrix&>
    PauliMomentMatrixFactory::operator()(MaintainsMutex::WriteLock &lock, const Index &index,
                                      Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_nearest_neighbour_moment_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{offset, matrix};
    }

    void PauliMomentMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const Index& index, ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_moment_matrix(lock, index, offset, matrix);
    }

    std::string PauliMomentMatrixFactory::not_found_msg(const PauliMomentMatrixFactory::Index &index) const {
        std::stringstream errSS;
        errSS << "Could not find moment matrix for level " << index.moment_matrix_level;
        if (index.neighbours > 0 ) {
            errSS << ", restricted to " << index.neighbours
                  << " nearest neighbour" << ((index.neighbours != 1) ? "s" : "");
        }
        if (index.wrapped) {
            errSS << " with wrapping";
        }
        errSS << ".";
        return errSS.str();
    }

    MaintainsMutex::WriteLock PauliMomentMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }


}
