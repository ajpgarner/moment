/**
 * pauli_localizing_matrix_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_localizing_matrix_indices.h"
#include "pauli_matrix_system.h"
#include "matrix/monomial_matrix.h"
#include "scenarios/context.h"

namespace Moment::Pauli {
    PauliLocalizingMatrixFactory::PauliLocalizingMatrixFactory(MatrixSystem& system)
            : PauliLocalizingMatrixFactory{dynamic_cast<PauliMatrixSystem&>(system)} { }


    std::pair<ptrdiff_t, MonomialMatrix&>
    PauliLocalizingMatrixFactory::operator()(MaintainsMutex::WriteLock &lock, const Index &index,
                                         Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_nearest_neighbour_localizing_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{offset, matrix};
    }

    void PauliLocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const Index& index, ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_localizing_matrix(lock, index, offset, matrix);
    }

    std::string PauliLocalizingMatrixFactory::not_found_msg(const PauliLocalizingMatrixFactory::Index &index) const {
        std::stringstream errSS;
        errSS << "Could not find localizing matrix of level " << index.Index.moment_matrix_level;
        errSS << " for sequence \"" << this->system.Context().format_sequence(index.Word);
        if (index.Index.neighbours > 0 ) {
            errSS << ", restricted to " << index.Index.neighbours
                  << " nearest neighbour" << ((index.Index.neighbours != 1) ? "s" : "");
        }
        if (index.Index.wrapped) {
            errSS << " with wrapping";
        }
        errSS << ".";
        return errSS.str();
    }

    MaintainsMutex::WriteLock PauliLocalizingMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }


}
