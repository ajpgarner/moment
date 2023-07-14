/**
 * polynomial_index_storage.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_index_storage.h"

#include "symbolic/polynomial_factory.h"

#include "matrix_system/matrix_indices.h"

#include <map>

namespace Moment {
    PolynomialIndexStorage::PolynomialIndexStorage(const PolynomialFactory &factory)
            : factoryPtr{&factory},
              ordering_functor{&factory},
              poly_index_map(std::make_unique<underlying_map_t>(ordering_functor)) {
    }

    PolynomialIndexStorage::PolynomialIndexStorage()
            : factoryPtr{nullptr}, ordering_functor{nullptr}, poly_index_map(nullptr) {
    }

    void PolynomialIndexStorage::set_factory(const PolynomialFactory &factory) {
        if (poly_index_map && !this->poly_index_map->empty()) [[unlikely]] {
            throw std::runtime_error{"Cannot change polynomial factory for non-empty indices."};
        }

        this->factoryPtr = &factory;
        this->ordering_functor.set_factory(factory);
        this->poly_index_map = std::make_unique<underlying_map_t>(this->ordering_functor);
    }


    std::pair<ptrdiff_t, bool> PolynomialIndexStorage::insert(const PolynomialLMIndex &poly, ptrdiff_t offset) {
        assert(this->factoryPtr);
        assert(this->poly_index_map);
        auto [iter, did_insert] = this->poly_index_map->try_emplace(poly, offset);
        return {iter->second, did_insert};
    }

    ptrdiff_t PolynomialIndexStorage::find(const PolynomialLMIndex& poly) const noexcept {
        assert(this->factoryPtr);
        assert(this->poly_index_map);
        auto where_iter = this->poly_index_map->find(poly);
        if (where_iter == this->poly_index_map->cend()) {
            return -1;
        }
        return where_iter->second;
    }

    static_assert(Moment::stores_indices<PolynomialIndexStorage, PolynomialLMIndex>);


}