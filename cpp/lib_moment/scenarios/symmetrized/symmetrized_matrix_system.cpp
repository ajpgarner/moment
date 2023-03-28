/**
 * symmetrized_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symmetrized_matrix_system.h"
#include "group.h"

#include "symmetrized_context.h"

#include <cstdio>


namespace Moment::Symmetrized {

    // TODO: Make context

    namespace {
        std::unique_ptr<Context> make_symmetrized_context(const MatrixSystem& source, const Group& group) {
            return std::make_unique<SymmetrizedContext>(source.Context());
        }
    }

    SymmetrizedMatrixSystem::SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem> &&base_system,
                                                     std::unique_ptr<Group>&& group)
        : MatrixSystem{make_symmetrized_context(*base_system, *group)},
            base_ms_ptr(std::move(base_system)), symmetry{std::move(group)} {

    }

    SymmetrizedMatrixSystem::~SymmetrizedMatrixSystem() noexcept = default;
}