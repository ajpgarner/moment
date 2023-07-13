/**
 * symmetrized_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system/matrix_system.h"
#include "../derived/derived_matrix_system.h"

#include <memory>

namespace Moment {
    class ExplicitSymbolIndex;
    namespace Derived {
        class MapCoreProcessor;
        class SymbolTableMap;
    }
}

namespace Moment::Symmetrized {
    class Group;

    class SymmetrizedMatrixSystem : public Derived::DerivedMatrixSystem {

    public:
        class SymmetrizedSTMFactory : public STMFactory {
        private:
            Group& group;
            const size_t max_word_length;
            std::unique_ptr<Derived::MapCoreProcessor> processor;

        public:
            SymmetrizedSTMFactory(Group &group, size_t max_word_length,
                                  std::unique_ptr<Derived::MapCoreProcessor>&& processor) noexcept;

            std::unique_ptr<Derived::SymbolTableMap> operator()(const SymbolTable& origin, SymbolTable& target) final;

        };

    private:
        /** Symmetry group defining the system */
        std::unique_ptr<Group> symmetry;

        /** Maxmimum word length that can be translated. */
        const size_t max_word_length;

    public:
        SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem> &&base_system,
                                std::unique_ptr<Group>&& group,
                                size_t max_word_length,
                                std::unique_ptr<Derived::MapCoreProcessor>&& processor);

        ~SymmetrizedMatrixSystem() noexcept override;

        [[nodiscard]] inline Group& group() noexcept{
            return *symmetry;
        }

        size_t longest_supported_word() const noexcept override {
            return this->max_word_length;
        }

        [[nodiscard]] inline const Group& group() const noexcept {
            return *symmetry;
        }

        [[nodiscard]] std::string system_type_name() const override {
            return "Symmetrized Matrix System";
        }

    };
}