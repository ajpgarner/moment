/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_matrix_system.h"
#include "pauli_context.h"

namespace Moment::Pauli {
    PauliMatrixSystem::PauliMatrixSystem(std::unique_ptr<class PauliContext> contextPtr, const double tolerance)
        : MatrixSystem{std::move(contextPtr), tolerance},
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())} {
    }

    PauliMatrixSystem::PauliMatrixSystem(const oper_name_t qubit_count, const double tolerance)
        : MatrixSystem(std::make_unique<PauliContext>(qubit_count), tolerance),
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())} {

    }

    PauliMatrixSystem::~PauliMatrixSystem() = default;

}