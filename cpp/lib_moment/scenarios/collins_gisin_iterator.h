/**
 * collins_gisin_iterator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "collins_gisin.h"

#include "utilities/multi_dimensional_offset_index_iterator.h"

#include <optional>

namespace Moment {
    /**
     * Iterator over elements in CG class.
     * Iterator must not be shared between threads!
     */
    class CollinsGisinIterator {
    private:
        const CollinsGisin* collinsGisinPtr;

        /** Evaluated current entry (only in virtual mode). */
        mutable std::optional<CollinsGisinEntry> current_entry;

        MultiDimensionalOffsetIndexIterator<true, CollinsGisinIndex> mdoii;
        size_t current_offset;

    public:
        /**
         * Construct iterator over range.
         */
        CollinsGisinIterator(const CollinsGisin& cg, CollinsGisinIndex&& first, CollinsGisinIndex&& last);

        /**
         * 'End' iterator constructor.
         */
        explicit CollinsGisinIterator(const CollinsGisin& cg);


        /**
         * Increment iterator.
         */
        CollinsGisinIterator& operator++() noexcept;

        /**
         * True, if iterator is not done.
         */
        [[nodiscard]] inline explicit operator bool() const noexcept {
            return static_cast<bool>(this->mdoii);
        }

        /**
         * True, if iterator is done.
         */
        [[nodiscard]] inline bool operator!() const noexcept {
            return !this->mdoii;
        }

        /**
         * Gets current CG index.
         */
        [[nodiscard]] inline const CollinsGisinIndex& index() const noexcept {
            return *this->mdoii;
        }

         /**
         * Gets current CG index.
         */
        [[nodiscard]] inline const CollinsGisinEntry& operator*() const;

        /**
         * Equality test.
         */
        [[nodiscard]] inline bool operator==(const CollinsGisinIterator& other) const noexcept {
            return this->mdoii == other.mdoii;
        }

        /**
         * Inequality test.
         */
        [[nodiscard]] inline bool operator!=(const CollinsGisinIterator& other) const noexcept {
            return this->mdoii != other.mdoii;
        }

        /**
         * Gets offset within splice represented by this iterator.
         */
        [[nodiscard]] inline const size_t block_index() const noexcept {
            return mdoii.global();
        }

        /**
         * Gets offset within CG tensor.
         */
        [[nodiscard]] inline const size_t offset() const noexcept {
            return this->current_offset;
        }

        /**
         * Pointed to operator sequence.
         */
        [[nodiscard]] const OperatorSequence& sequence() const;

        /**
         * Pointed to symbol ID, if known.
         */
        [[nodiscard]] symbol_name_t symbol_id() const;

        /**
         * Pointed to real basis element, if known.
         */
        [[nodiscard]] ptrdiff_t real_basis() const;

    private:
        void make_cached_entry() const;
    };

    class CollinsGisinRange {
    private:
        const CollinsGisin& collinsGisin;
        const CollinsGisinIndex first;
        const CollinsGisinIndex last;

        const CollinsGisinIterator iter_end;

    public:
        CollinsGisinRange(const CollinsGisin& cg, CollinsGisinIndex&& first, CollinsGisinIndex&& last)
            : collinsGisin{cg}, first(std::move(first)), last(std::move(last)),
              iter_end(cg) {
        }

        [[nodiscard]] CollinsGisinIterator begin() const {
            return CollinsGisinIterator{collinsGisin, CollinsGisinIndex(this->first), CollinsGisinIndex(this->last)};
        }

        [[nodiscard]] inline const CollinsGisinIterator& end() const noexcept {
            return this->iter_end;
        }

    };
}