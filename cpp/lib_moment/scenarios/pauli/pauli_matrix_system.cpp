/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_matrix_system.h"
#include "pauli_context.h"
#include "pauli_moment_matrix.h"

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

    std::unique_ptr<SymbolicMatrix>
    PauliMatrixSystem::create_moment_matrix(MaintainsMutex::WriteLock& lock, size_t level,
                                            Multithreading::MultiThreadPolicy mt_policy) {
        return this->create_nearest_neighbour_moment_matrix(lock, NearestNeighbourIndex{level, 0, false}, mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_moment_matrix(MaintainsMutex::WriteLock& write_lock,
                                                              const PauliMomentMatrixIndex& index,
                                                              Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = PauliMomentMatrix::create_matrix(this->pauliContext, symbol_table, index, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
        //assert(this->is_locked_write_lock(write_lock));
        //return PauliMomentMatrix::create_matrix(this->pauliContext, this->Symbols(), index, mt_policy);
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
                                                                   const MonomialMatrix& mm) {
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