/**
 * symmetrized_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symmetrized_matrix_system.h"
#include "group.h"

#include "symmetrized_context.h"

#include <cassert>

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

        // Avoid deadlock. Should never occur...!
        assert(this->base_ms_ptr.get() != this);



    }

    SymmetrizedMatrixSystem::~SymmetrizedMatrixSystem() noexcept = default;

    std::unique_ptr<struct MomentMatrix> SymmetrizedMatrixSystem::createNewMomentMatrix(size_t level) {
        // First of all, make sure we have rep. up to the right level
        const auto& rep = this->symmetry->representation(2*level);

        throw std::logic_error{"SymmetrizedMatrixSystem::createNewMomentMatrix not yet implemented."};
    }

    std::unique_ptr<struct LocalizingMatrix>
    SymmetrizedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) {
        // First of all, make sure we have rep. up to the right level
        const auto& rep = this->symmetry->representation(2*lmi.Level + lmi.Word.size());

        throw std::logic_error{"SymmetrizedMatrixSystem::createNewLocalizingMatrix not yet implemented."};
    }

}