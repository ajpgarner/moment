/**
 * wordlist.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "word_list.h"

#include "symbolic/symbol_table.h"

#include <atomic>
#include <ranges>
#include <stdexcept>
#include <sstream>

namespace Moment {
    WordList::WordList(const Context& context) : context{context}, symbol_map_max_length{0} {
        // Initially, all we know is that index 0 of generator maps to symbol 1 (e).
        this->symbol_map.emplace_back(1);
    }

    const OperatorSequenceGenerator& WordList::operator[](size_t word_length) const {
        // Try and read
        std::shared_lock lock{this->mutex};
        if (word_length < this->osgs.size()) {
            auto& ptr = this->osgs[word_length];
            if (ptr) {
                return *ptr; // release read lock
            }
        }
        lock.unlock();

        // Not found, try to write
        std::unique_lock write_lock{this->mutex};

        // Check wasn't created by racing other thread
        std::atomic_thread_fence(std::memory_order_acquire);
        const bool expansion_required = word_length >= this->osgs.size();
        if (!expansion_required) {
            auto& ptr = this->osgs[word_length];
            if (ptr) {
                return *ptr; // release write lock
            }
        }

        // Create
        if (expansion_required) {
            this->osgs.resize(word_length+1);
            this->conj_osgs.resize(word_length+1);
        }
        this->osgs[word_length] = std::make_unique<OperatorSequenceGenerator>(context, 0, word_length);

        // Create conjugate
        auto conjed = this->osgs[word_length]->conjugate();
        this->conj_osgs[word_length] = std::make_unique<OperatorSequenceGenerator>(std::move(conjed));

        // Release fence
        std::atomic_thread_fence(std::memory_order_release);

        // Return newly created OSG
        return *this->osgs[word_length]; // release write lock
    }

    const OperatorSequenceGenerator& WordList::conjugated(size_t word_length) const {
        // Check generation using []
        const auto& x = (*this)[word_length];

        assert(word_length < this->conj_osgs.size());
        assert(this->conj_osgs[word_length]);

        return *(this->conj_osgs[word_length]);
    }

    bool WordList::update_symbol_map(const SymbolTable& table) {
        std::shared_lock read_lock{this->mutex};
        // Find largest created OSG
        const size_t largest_osg = [&]() {
            for (const auto& osg : std::ranges::reverse_view(osgs)) {
                if (osg) {
                    return osg->max_sequence_length;
                }
            }
            return 0ULL;
        }();

        // If largest OSG already generated, return.
        if (largest_osg <= this->symbol_map_max_length.load(std::memory_order_acquire)) {
            return false; // unlock read lock
        }

        // Upgrade lock
        read_lock.unlock();
        std::shared_lock write_lock{this->mutex};
        if (largest_osg <= this->symbol_map_max_length.load(std::memory_order_acquire)) {
            return false;
        }
        const auto& new_osg = *this->osgs[largest_osg];

        const auto target_size = new_osg.size();
        const auto start_index = this->symbol_map.size();

        this->symbol_map.reserve(target_size);
        for (auto iter = new_osg.begin() + static_cast<ptrdiff_t>(start_index); iter != new_osg.end(); ++iter) {
            const auto& seq = *iter;
            const auto* datum = table.where(seq);
            assert(datum != nullptr);
            const bool isConjugated = (seq != datum->sequence());
            this->symbol_map.emplace_back((isConjugated ? -1 : 1) * datum->Id());
        }

        assert(this->symbol_map.size() == new_osg.size());

        // Update atomic
        this->symbol_map_max_length.store(new_osg.max_sequence_length, std::memory_order_release);

        // Release write lock~
        return true;
    }

    std::pair<symbol_name_t, bool> WordList::osg_index_to_symbol(size_t index) const {
        std::shared_lock read_lock{this->mutex};
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