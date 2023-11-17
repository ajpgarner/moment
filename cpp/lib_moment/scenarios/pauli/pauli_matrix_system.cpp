/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_matrix_system.h"
#include "pauli_context.h"

#include <cassert>

namespace Moment::Pauli {
    PauliMatrixSystem::PauliMatrixSystem(std::unique_ptr<class PauliContext> contextPtr, const double tolerance)
        : MatrixSystem{std::move(contextPtr), tolerance},
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())},
          PauliMomentMatrices{*this} {
    }

    PauliMatrixSystem::PauliMatrixSystem(const oper_name_t qubit_count, const double tolerance)
        : MatrixSystem(std::make_unique<PauliContext>(qubit_count), tolerance),
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())},
          PauliMomentMatrices{*this} {

    }

    std::unique_ptr<PauliMomentMatrix>
    PauliMatrixSystem::create_nearest_neighbour_moment_matrix(MaintainsMutex::WriteLock& lock,
                                                              const PauliMomentMatrixIndex& index,
                                                              Multithreading::MultiThreadPolicy mt_policy) {
        // TODO: Create NN MM
        throw std::logic_error{"PauliMatrixSystem::create_nearest_neighbour_moment_matrix not implemented."};
    }

    void PauliMatrixSystem::on_new_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                 size_t moment_matrix_level,
                                                 ptrdiff_t matrix_offset,
                                                 const SymbolicMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        PauliMomentMatrixIndex pmmi{moment_matrix_level, 0, false};
        assert(!this->PauliMomentMatrices.contains(pmmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliMomentMatrices.insert_alias(write_lock, pmmi, matrix_offset);
        assert(actual_offset == matrix_offset);

    }

    void PauliMatrixSystem::on_new_nearest_neighbour_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                   const PauliMomentMatrixIndex& index,
                                                                   ptrdiff_t matrix_offset,
                                                                   const PauliMomentMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain MM index if unrestricted
        if (0 == index.neighbours) {
            assert(!this->MomentMatrix.contains(index.moment_matrix_level));
            [[maybe_unused]] auto actual_offset =
                    this->MomentMatrix.insert_alias(write_lock, index.moment_matrix_level, matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }

    PauliMatrixSystem::~PauliMatrixSystem() = default;

}