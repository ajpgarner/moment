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
    namespace Derived {
        class SymbolTableMap;
    }
}

namespace Moment::Symmetrized {
    class Group;

    class SymmetrizedMatrixSystem : public MatrixSystem {

    private:
        /**
         * Owning pointer to base system.
         * Ownership is necessary, to prevent deletion of base system while SMS is still alive.
         */
        std::shared_ptr<MatrixSystem> base_ms_ptr;

        /** Symmetry group defining the system */
        std::unique_ptr<Group> symmetry;

        /** Map that defines the system */
        std::unique_ptr<Derived::SymbolTableMap> map_ptr;

    public:
        SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem> &&base_system, std::unique_ptr<Group>&& group);

        ~SymmetrizedMatrixSystem() noexcept override;

        [[nodiscard]] inline MatrixSystem& base_system() noexcept {
            return *base_ms_ptr;
        }

        [[nodiscard]] inline const MatrixSystem& base_system() const noexcept {
            return *base_ms_ptr;
        }

        [[nodiscard]] inline Group& group() noexcept{
            return *symmetry;
        }

        [[nodiscard]] inline const Group& group() const noexcept {
            return *symmetry;
        }

        [[nodiscard]] inline const Derived::SymbolTableMap& map() const noexcept {
            return *map_ptr;
        }

        [[nodiscard]] std::string system_type_name() const override {
            return "Symmetrized Matrix System";
        }

    protected:
        std::unique_ptr<struct MomentMatrix> createNewMomentMatrix(size_t level) override;

        std::unique_ptr<struct LocalizingMatrix> createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) override;
    };
}