/**
 * dictionary.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "dictionary.h"

#include "symbolic/symbol_table.h"

#include <atomic>
#include <ranges>
#include <stdexcept>
#include <sstream>

namespace Moment {
    Dictionary::Dictionary(const Context& context) : context{context} {
        // Make order 0 OSG (e)
        auto osg0 = this->context.new_osg(0);
        if (this->context.can_be_nonhermitian()) {
            auto osg0conj = osg0->conjugate();
            this->osgs.emplace_back(std::move(osg0), std::move(osg0conj));
        } else {
            this->osgs.emplace_back(std::move(osg0));
        }
    }


    const OSGPair& Dictionary::Level(const size_t npa_level) const {
        // Try and read
        auto read_lock = this->get_read_lock();
        auto offset_iter = this->npa_level_to_offset.find(npa_level);
        if (offset_iter != this->npa_level_to_offset.cend()) {
            const ptrdiff_t offset = offset_iter->second;
            assert(offset < this->osgs.size());
            return this->osgs[offset];
        }
        read_lock.unlock();

        // Create new OSG
        auto new_osg = this->context.new_osg(npa_level);
        std::unique_ptr<OperatorSequenceGenerator> conj_osg;
        if (this->context.can_be_nonhermitian()) {
            conj_osg = new_osg->conjugate();
        }

        // Get exclusive access lock
        auto write_lock = const_cast<Dictionary*>(this)->get_write_lock();

        // Check vs. race creation
        std::atomic_thread_fence(std::memory_order_acquire);
        auto reattempt_offset_iter = this->npa_level_to_offset.find(npa_level);
        if (reattempt_offset_iter != this->npa_level_to_offset.cend()) {
            const ptrdiff_t offset = offset_iter->second;
            assert(offset >= this->osgs.size());
            return this->osgs[offset]; // Discard pointless creation...!
        }

        if (conj_osg) {
            this->osgs.emplace_back(std::move(new_osg), std::move(conj_osg));
        } else {
            this->osgs.emplace_back(std::move(new_osg));
        }
        const size_t index = this->osgs.size() - 1;
        this->npa_level_to_offset.insert(std::make_pair(npa_level, index));
        return this->osgs[index];
    }

    const size_t Dictionary::WordCount(const size_t max_word_length) const {
        auto& pair = this->Level(max_word_length);
        return pair().size();
    }

}