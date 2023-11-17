/**
 * rulebook_storage.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "rulebook_storage.h"

#include "matrix_system.h"

#include "symbolic/symbol_table.h"
#include "symbolic/rules/moment_rulebook.h"

#include <sstream>

namespace Moment {

    RulebookStorage::RulebookStorage(MatrixSystem &system) : system{system} { }

    RulebookStorage::~RulebookStorage() noexcept = default;

    std::pair<size_t, MomentRulebook&>
    RulebookStorage::add(const MaintainsMutex::WriteLock& write_lock,
                         std::unique_ptr<MomentRulebook>&& input_rulebook_ptr) {
        assert(write_lock.owns_lock());
        assert(&input_rulebook_ptr->symbols == this->system.symbol_table.get());

        // Exception safe insert:
        this->rulebooks.emplace_back(nullptr);
        this->rulebooks.back().swap(input_rulebook_ptr);

        try {
            // Get info
            size_t rulebook_index = this->rulebooks.size() - 1;
            auto &rulebook = *this->rulebooks[rulebook_index];

            // Check if rulebook has a name, set default name otherwise.
            if (rulebook.name().empty()) {
                std::stringstream nameSS;
                nameSS << "Rulebook #" << rulebook_index;
                rulebook.set_name(nameSS.str());
            }

            // Dispatch notification to derived classes
            this->system.on_rulebook_added(write_lock, rulebook_index, rulebook, true);

            // Return created book
            return {rulebook_index, rulebook};
        } catch (const std::exception& e) {
            //If throwing an exception, return rulebook to input_rulebook_ptr object
            if (!this->rulebooks.empty()) {
                this->rulebooks.back().swap(input_rulebook_ptr);
                this->rulebooks.pop_back();
            }
            throw;
        }
    }

    std::pair<size_t, MomentRulebook &> RulebookStorage::add(std::unique_ptr<MomentRulebook>&& rulebook) {
        auto write_lock = this->system.get_write_lock();
        return this->add(write_lock, std::move(rulebook));
    }

    std::pair<size_t, MomentRulebook &>
    RulebookStorage::merge_in(const MaintainsMutex::WriteLock& write_lock,
                              const size_t existing_rulebook_id, MomentRulebook&& input_rulebook) {
        assert(write_lock.owns_lock());
        auto& existing_rulebook = this->find(existing_rulebook_id);

        existing_rulebook.combine_and_complete(std::move(input_rulebook));

        // NB: Name should be handled already, either from existing name, or newly-merged-in name.

        // Dispatch notification of merge-in to derived classes
        this->system.on_rulebook_added(write_lock, existing_rulebook_id, existing_rulebook, false);

        // Return merged book
        return {existing_rulebook_id, existing_rulebook};
    }

    std::pair<size_t, MomentRulebook &>
    RulebookStorage::merge_in(const size_t existing_rulebook_id, MomentRulebook&& input_rulebook) {
        auto write_lock = this->system.get_write_lock();
        return this->merge_in(write_lock, existing_rulebook_id, std::move(input_rulebook));
    }

    void RulebookStorage::refreshAll(const MaintainsMutex::WriteLock& write_lock, const size_t previous_symbol_count) {
        assert(write_lock.owns_lock());
        for (auto& bookPtr : this->rulebooks) {
            this->system.expand_rulebook(*bookPtr, previous_symbol_count);
        }
    }

    MomentRulebook& RulebookStorage::find(size_t index) {
        if (index >= this->rulebooks.size()) {
            throw errors::missing_component("Rulebook index out of range.");
        }
        if (!this->rulebooks[index]) {
            throw errors::missing_component("Rulebook at supplied index was missing.");
        }
        return *this->rulebooks[index];
    }

    const MomentRulebook& RulebookStorage::find(size_t index) const {
        if (index >= this->rulebooks.size()) {
            throw errors::missing_component("Rulebook index out of range.");
        }
        if (!this->rulebooks[index]) {
            throw errors::missing_component("Rulebook at supplied index was missing.");
        }
        return *this->rulebooks[index];
    }
}