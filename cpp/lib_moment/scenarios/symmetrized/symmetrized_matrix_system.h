/**
 * symmetrized_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system.h"

#include <memory>

namespace Moment {
    class ExplicitSymbolIndex;
}

namespace Moment::Symmetrized {
    class Group;

    class SymmetrizedMatrixSystem : public MatrixSystem {

    private:
        std::shared_ptr<MatrixSystem> base_ms_ptr;
        std::unique_ptr<Group> symmetry;

    public:
        SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem> &&base_system, std::unique_ptr<Group>&& group);

        ~SymmetrizedMatrixSystem() noexcept override;

        MatrixSystem &base_system() {
            return *base_ms_ptr;
        }

        const MatrixSystem &base_system() const {
            return *base_ms_ptr;
        }

        Group& group() {
            return *symmetry;
        }

        const Group& group() const {
            return *symmetry;
        }

        std::string system_type_name() const override {
            return "Symmetrized Matrix System";
        }

    protected:
        std::unique_ptr<struct MomentMatrix> createNewMomentMatrix(size_t level) override;

        std::unique_ptr<struct LocalizingMatrix> createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) override;
    };
}