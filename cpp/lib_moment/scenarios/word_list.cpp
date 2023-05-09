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
    WordList::WordList(const Context& context) : context{context} {
        // Make order 0 OSG (e)
        this->osgs.emplace_back(this->context.new_osg(0));
        this->conj_osgs.emplace_back(std::make_unique<OperatorSequenceGenerator>(this->osgs[0]->conjugate()));
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
        this->osgs[word_length] = this->context.new_osg(word_length);

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


    const OperatorSequenceGenerator& WordList::largest() const {
        // Find largest created OSG
        for (const auto &osg: std::ranges::reverse_view(osgs)) {
            if (osg) {
                return *osg;
            }
        }
        throw std::runtime_error{"WordList contains no OperatorSequenceGenerators!"};
    }


}