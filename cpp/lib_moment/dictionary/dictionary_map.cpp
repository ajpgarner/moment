/**
 * dictionary_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "dictionary_map.h"

#include "scenarios/context.h"
#include "dictionary.h"

#include "symbolic/symbol_table.h"

#include <ranges>
#include <sstream>

namespace Moment {
    DictionaryMap::DictionaryMap(const Context& context, const SymbolTable& symbols)
        : context{context}, symbols{symbols}, symbol_map_max_length{0} {
        // Initially, all we know is that index 0 of generator maps to symbol 1 (e).
        this->symbol_map.emplace_back(1);
    }

    bool DictionaryMap::update_if_necessary(size_t desired_length) {
        // No need to update if length is already long enough
        if (desired_length <= this->symbol_map_max_length.load(std::memory_order_acquire)) {
            return false;
        }

        // Lock for write, and check for race condition
        std::unique_lock write_lock = this->get_write_lock();
        if (desired_length <= this->symbol_map_max_length.load(std::memory_order_acquire)) {
            return false; // Nothing more to do!
        }

        // Get long-enough OSG
        const auto& promised_osg = this->context.operator_sequence_generator(desired_length);
        const auto target_size = promised_osg.size();
        const auto start_index = this->symbol_map.size();

        this->symbol_map.reserve(target_size);
        for (auto iter = promised_osg.begin() + static_cast<ptrdiff_t>(start_index); iter != promised_osg.end(); ++iter) {
            const auto& seq = *iter;
            const auto datum = symbols.where(seq);
            assert(datum.found());
            const bool isConjugated = (seq != datum->sequence());
            this->symbol_map.emplace_back((isConjugated ? -1 : 1) * datum->Id());
        }

        assert(this->symbol_map.size() == promised_osg.size());

        // Update atomic
        this->symbol_map_max_length.store(promised_osg.max_sequence_length, std::memory_order_release);

        // Release write lock~
        return true;
    }


    std::pair<symbol_name_t, bool> DictionaryMap::operator()(size_t index) const {
        std::shared_lock read_lock = this->get_read_lock();
        if (index >= this->symbol_map.size()) {
            std::stringstream errSS;
            errSS << "Symbol at index " << index << " not yet defined.";
            throw std::range_error{errSS.str()};
        }

        // Return symbol
        auto val = this->symbol_map[index];
        if (val < 0) {
            return {-val, true};
        }
        return {val, false};
    }
}