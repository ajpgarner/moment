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

            std::unique_ptr<Derived::SymbolTableMap> make(const SymbolTable& origin,
                                                          SymbolTable& target,
                                                          Multithreading::MultiThreadPolicy mt_policy) final;

        };

    private:
        /** Symmetry group defining the system */
        std::unique_ptr<Group> symmetry;

        /** Maxmimum word length that can be translated. */
        const size_t max_word_length;

    public:
        /**
         * Creates a SymmetrizedMatrixSystem
         * @param base_system The source system to symmetrize.
         * @param group The symmetry group.
         * @param max_word_length The longest operator string that can be mapped.
         * @param processor The class for finding the map between groups.
         * @param zero_tolerance The tolerance to which expressions are treated as zero (-1 for base system value).
         */
        SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem> &&base_system,
                                std::unique_ptr<Group>&& group,
                                size_t max_word_length,
                                std::unique_ptr<Derived::MapCoreProcessor>&& processor,
                                double zero_tolerance = -1.0,
                                Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

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