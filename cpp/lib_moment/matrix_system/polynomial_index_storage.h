/**
 * polynomial_index_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial_localizing_matrix_index.h"

#include "symbolic/polynomial_ordering.h"

#include "integer_types.h"

#include <map>
#include <memory>
#include <optional>

namespace Moment {
    class PolynomialFactory;

    class PolynomialIndexStorage {
    public:
        class IndexPolyComparator {
        private:
            PolynomialOrderingWithCoefficients poly_comp;

            inline void set_factory(const PolynomialFactory& factory) {
                this->poly_comp.set_factory(factory);
            }

        public:
            explicit IndexPolyComparator(const PolynomialFactory* factoryPtr = nullptr)
                    : poly_comp{factoryPtr} { }


            [[nodiscard]] bool operator()(const PolynomialLMIndex& lhs, const PolynomialLMIndex& rhs) const noexcept {
                if (lhs.Level < rhs.Level) {
                    return true;
                } else if (lhs.Level > rhs.Level) {
                    return false;
                }
                return poly_comp(lhs.Polynomial, rhs.Polynomial);
            }

            friend class PolynomialIndexStorage;
        };

    private:
        const PolynomialFactory* factoryPtr;
        IndexPolyComparator ordering_functor;
        std::map<PolynomialLMIndex, ptrdiff_t, IndexPolyComparator> poly_index_map;

    public:
        PolynomialIndexStorage();

        explicit PolynomialIndexStorage(const PolynomialFactory& factory);

        /**
         * Sets the polynomial factory.
         */
         void set_factory(const PolynomialFactory& factory);

        /**
         * Attempts to insert an index, or returns existing offset.
         * @param poly
         * @param index
         * @return Pair, first: offset, second: true if insertion took place, false if existing index returned.
         */
         std::pair<ptrdiff_t, bool> insert(const PolynomialLMIndex& level_poly, ptrdiff_t offset);

        /**
         * Compare two keys
         */
        [[nodiscard]] inline bool less(const PolynomialLMIndex& lhs, const PolynomialLMIndex& rhs) const noexcept {
            return this->ordering_functor(lhs, rhs);
        }

        /**
         * Gets the index associated with a polynomial, if any exists.
         */
        [[nodiscard]] ptrdiff_t find(const PolynomialLMIndex& level_poly) const noexcept;

        /**
         * Tests if a polynomial has an index.
         */
        [[nodiscard]] inline bool contains(const PolynomialLMIndex& level_poly) const noexcept {
            const auto result = this->find(level_poly);
            return result >= 0;
        }

         /**
          * Are there any indices recorded?
          */
         [[nodiscard]] inline bool empty() const noexcept {
             return this->poly_index_map.empty();
         }

         /**
          * How many indices are recorded?
          */
         [[nodiscard]] inline size_t size() const noexcept {
             return this->poly_index_map.size();
         }

    };

}