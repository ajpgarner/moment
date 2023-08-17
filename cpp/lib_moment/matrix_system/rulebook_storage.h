/**
 * rulebook_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading/maintains_mutex.h"

#include <memory>
#include <vector>

namespace Moment {
    class MatrixSystem;
    class MomentRulebook;

    class RulebookStorage {
    private:
        MatrixSystem& system;

        /** List of moment substitution rulebooks in the system. */
        std::vector<std::unique_ptr<MomentRulebook>> rulebooks;

    public:
        explicit RulebookStorage(MatrixSystem& system);


        ~RulebookStorage() noexcept;

        /**
          * Import a list of moment substitution rules.
          */
        [[nodiscard]] std::pair<size_t, MomentRulebook&> add(const MaintainsMutex::WriteLock& lock,
                                                             std::unique_ptr<MomentRulebook>&& rulebook);

        /**
          * Import a list of moment substitution rules.
          * Will lock until all read locks have expired - so do NOT first call for a read lock...!
          */
        [[nodiscard]] std::pair<size_t, MomentRulebook&> add(std::unique_ptr<MomentRulebook>&& rulebook);

        /**
         * Import a list of moment substitution rules
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @throws errors::missing_component If existing_rulebook_id does not correspond to a valid rulebook.
         */
        std::pair<size_t, MomentRulebook&>
        merge_in(const MaintainsMutex::WriteLock& lock,
                 size_t existing_rulebook_id, MomentRulebook&& rulebook);

        /**
         * Import a list of moment substitution rules
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @throws errors::missing_component If existing_rulebook_id does not correspond to a valid rulebook.
         */
        std::pair<size_t, MomentRulebook&>
        merge_in(size_t existing_rulebook_id, MomentRulebook&& rulebook);

        /**
         * Update all rulebook with new symbols
         */
        void refreshAll(const MaintainsMutex::WriteLock& write_lock, size_t previous_symbol_count);

        /**
         * Get a list of moment substitution rules.
         * For thread safety, call for a read lock first.
         * @throws errors::missing_component If index does not correspond to a valid rulebook.
         */
        [[nodiscard]] MomentRulebook& find(size_t index);

        /**
         * Get a list of moment substitution rules
         * For thread safety, call for a read lock first.
         * @throws errors::missing_component If index does not correspond to a valid rulebook.
         */
        [[nodiscard]] const MomentRulebook& find(size_t index) const;

        /**
         * True if rulebook can be found at specified index
         */
        [[nodiscard]] bool contains(size_t index) const noexcept {
            return (index < this->rulebooks.size()) && (this->rulebooks[index].operator bool());
        }

        /**
         * Counts number of rulebooks in system
         */
        [[nodiscard]] size_t size() const noexcept { return this->rulebooks.size(); }

        /**
         * Counts number of rulebooks in system
         */
        [[nodiscard]] size_t empty() const noexcept { return this->rulebooks.empty(); }

        [[nodiscard]] inline MomentRulebook& operator()(const size_t index) {
            return this->find(index);
        }

        [[nodiscard]] inline const MomentRulebook& operator()(const size_t index) const {
            return this->find(index);
        }
    };
}