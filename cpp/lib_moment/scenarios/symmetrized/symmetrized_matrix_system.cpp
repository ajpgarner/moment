/**
 * symmetrized_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symmetrized_matrix_system.h"
#include "group.h"

#include "symmetrized_context.h"
#include "implied_map.h"

#include "matrix/moment_matrix.h"
#include "matrix/localizing_matrix.h"

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
        // NOTE: We should be an scope where we hold a write lock to this matrix system.

        // First check source moment matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class MomentMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_moment_matrix(level);
            if (index >= 0) {
                return dynamic_cast<const class MomentMatrix&>(this->base_system()[index]);
            }
            read_source_lock.unlock();

            // Wait for write lock...
            auto write_source_lock = this->base_system().get_write_lock();
            auto [mm_index, mm] = this->base_system().create_moment_matrix(level);

            return mm; // write_source_lock unlocks
        }();

        // TODO: If a larger moment matrix has already been created, no need to re-do generation steps

        // Next, ensure source scenario defines a sufficiently large symbol map
        this->base_system().generate_dictionary(2*level);

        // Next, ensure the symmetry group has a large enough representation
        const auto& rep = this->symmetry->representation(2*level);

        // Lock source for read again.
        auto source_lock = this->base_system().get_read_lock();

        // Create map
        ImpliedMap base_change{*this, rep};

        // Apply map




        throw std::logic_error{"SymmetrizedMatrixSystem::createNewMomentMatrix not yet implemented."};
    }

    std::unique_ptr<struct LocalizingMatrix>
    SymmetrizedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) {
        // First of all, make sure we have rep. up to the right level
        const auto& rep = this->symmetry->representation(2*lmi.Level + lmi.Word.size());

        throw std::logic_error{"SymmetrizedMatrixSystem::createNewLocalizingMatrix not yet implemented."};
    }

}