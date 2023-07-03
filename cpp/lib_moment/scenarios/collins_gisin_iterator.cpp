/**
 * collins_gisin_iterator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "collins_gisin_iterator.h"

namespace Moment {

    CollinsGisinIterator::CollinsGisinIterator(const CollinsGisin &cg,
                                               CollinsGisinIndex&& first,
                                               CollinsGisinIndex&& last)
           : collinsGisinPtr{&cg}, mdoii{std::move(first), std::move(last)} {
        if (this->mdoii) {
            this->current_offset = this->collinsGisinPtr->index_to_offset_no_checks(*this->mdoii);
        } else {
            this->current_offset = 0;
        }
    }


    CollinsGisinIterator::CollinsGisinIterator(const CollinsGisin &cg)
        : collinsGisinPtr{&cg}, mdoii{}, current_offset{0} {
        assert(!this->mdoii);
    }

    CollinsGisinIterator& CollinsGisinIterator::operator++() noexcept {
        ++this->mdoii;
        if (this->mdoii) {
            this->current_offset = this->collinsGisinPtr->index_to_offset_no_checks(*this->mdoii);
        } else {
            this->current_offset = 0;
        }
        return *this;
    }

    symbol_name_t CollinsGisinIterator::symbol_id() const {
        return this->collinsGisinPtr->symbols[this->current_offset];
    }

    ptrdiff_t CollinsGisinIterator::real_basis() const {
        return this->collinsGisinPtr->real_indices[this->current_offset];
    }

}