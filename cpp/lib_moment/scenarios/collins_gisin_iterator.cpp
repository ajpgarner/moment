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

    const CollinsGisinEntry& CollinsGisinIterator::operator*() const {
        if (this->collinsGisinPtr->StorageType == TensorStorageType::Explicit) {
            return this->collinsGisinPtr->data[this->current_offset];
        } else {
            this->make_cached_entry();
            return this->current_entry.value();
        }
    }

    CollinsGisinIterator& CollinsGisinIterator::operator++() noexcept {
        this->current_entry.reset();

        ++this->mdoii;
        if (this->mdoii) {
            this->current_offset = this->collinsGisinPtr->index_to_offset_no_checks(*this->mdoii);
        } else {
            this->current_offset = 0;
        }
        return *this;
    }

    const OperatorSequence& CollinsGisinIterator::sequence() const {
        if (this->collinsGisinPtr->StorageType == TensorStorageType::Explicit) {
            return this->collinsGisinPtr->data[this->current_offset].sequence;
        } else {
            this->make_cached_entry();
            return this->current_entry->sequence;
        }
    }

    symbol_name_t CollinsGisinIterator::symbol_id() const {
        if (this->collinsGisinPtr->StorageType == TensorStorageType::Explicit) {
            return this->collinsGisinPtr->data[this->current_offset].symbol_id;
        } else {
            this->make_cached_entry();
            return this->current_entry->symbol_id;
        }
    }

    ptrdiff_t CollinsGisinIterator::real_basis() const {
        if (this->collinsGisinPtr->StorageType == TensorStorageType::Explicit) {
            return this->collinsGisinPtr->data[this->current_offset].real_index;
        } else {
            this->make_cached_entry();
            return this->current_entry->real_index;
        }
    }

    void CollinsGisinIterator::make_cached_entry() const {
        assert(this->collinsGisinPtr->StorageType == TensorStorageType::Virtual);
        if (this->current_entry.has_value()) {
            return;
        }

        this->current_entry = this->collinsGisinPtr->make_element_no_checks(*this->mdoii);
    }
}