/**
 * polynomial_index_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "matrix_system/indices/polynomial_localizing_matrix_index.h"
#include "matrix_system/matrix_indices.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/polynomial_ordering.h"

#include "integer_types.h"

#include <cassert>

#include <map>
#include <memory>
#include <optional>

namespace Moment {

    template<typename base_index_t = size_t,
            typename element_index_t = LocalizingMatrixIndex>
    class PolynomialIndexStorageBase {
    public:
        using Index = PolynomialLMIndexBase<base_index_t, element_index_t>;

        class IndexPolyComparator {
        private:
            PolynomialOrderingWithCoefficients poly_comp;

            inline void set_factory(const PolynomialFactory& factory) & {
                this->poly_comp.set_factory(factory);
            }

        public:
            explicit IndexPolyComparator(const PolynomialFactory* factoryPtr = nullptr)
                    : poly_comp{factoryPtr} { }


            [[nodiscard]] bool operator()(const Index& lhs, const Index& rhs) const noexcept {
                if (lhs.Level < rhs.Level) {
                    return true;
                } else if (lhs.Level > rhs.Level) {
                    return false;
                }
                return poly_comp(lhs.Polynomial, rhs.Polynomial);
            }

            friend class PolynomialIndexStorageBase<base_index_t, element_index_t>;
        };

        using underlying_map_t = std::map<Index, ptrdiff_t, IndexPolyComparator>;

    private:
        PolynomialFactory const * factoryPtr = nullptr;
        IndexPolyComparator ordering_functor;
        std::unique_ptr<underlying_map_t> poly_index_map;

    public:
        PolynomialIndexStorageBase() = default;

        /**
         * Construct polynomial index storage with initial factory
         * @param factory
         */
        explicit PolynomialIndexStorageBase(const PolynomialFactory& factory)
            : factoryPtr{&factory}, ordering_functor{&factory},
                poly_index_map(std::make_unique<underlying_map_t>(ordering_functor)) {
            }


        /**
         * Changes the indexing polynomial factory.
         */
         void set_factory(const PolynomialFactory& factory) {
            if (poly_index_map && !this->poly_index_map->empty()) [[unlikely]] {
                throw std::runtime_error{"Cannot change polynomial factory for non-empty indices."};
            }

            this->factoryPtr = &factory;
            this->ordering_functor.set_factory(factory);
            this->poly_index_map = std::make_unique<underlying_map_t>(this->ordering_functor);
        }


        /**
         * Attempts to insert an index, or returns existing offset.
         * @param poly
         * @param index
         * @return Pair, first: offset, second: true if insertion took place, false if existing index returned.
         */
        std::pair<ptrdiff_t, bool> insert(const Index& poly, ptrdiff_t offset) {
            assert(this->factoryPtr);
            assert(this->poly_index_map);
            auto [iter, did_insert] = this->poly_index_map->try_emplace(poly, offset);
            return {iter->second, did_insert};
        }


        /**
         * Compare two keys
         */
        [[nodiscard]] inline bool less(const Index& lhs, const Index& rhs) const noexcept {
            return this->ordering_functor(lhs, rhs);
        }

        /**
         * Gets the index associated with a polynomial, if any exists.
         */
        [[nodiscard]] ptrdiff_t find(const Index& poly) const noexcept {
            assert(this->factoryPtr);
            assert(this->poly_index_map);
            auto where_iter = this->poly_index_map->find(poly);
            if (where_iter == this->poly_index_map->cend()) {
                return -1;
            }
            return where_iter->second;
        }


        /**
         * Tests if a polynomial has an index.
         */
        [[nodiscard]] inline bool contains(const Index& level_poly) const noexcept {
            const auto result = this->find(level_poly);
            return result >= 0;
        }

         /**
          * Are there any indices recorded?
          */
         [[nodiscard]] inline bool empty() const noexcept {
             return !this->poly_index_map || this->poly_index_map->empty();
         }

         /**
          * How many indices are recorded?
          */
         [[nodiscard]] inline size_t size() const noexcept {
             return this->poly_index_map ? this->poly_index_map->size() : 0;
         }

    };

    using PolynomialIndexStorage = PolynomialIndexStorageBase<size_t, LocalizingMatrixIndex>;
    static_assert(stores_indices<PolynomialIndexStorage, PolynomialLMIndex>);

}