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
            : factoryPtr{&factory}, ordering_functor{&factory}, poly_index_map(ordering_functor) {
    }

    PolynomialIndexStorage::PolynomialIndexStorage()
            : factoryPtr{nullptr}, ordering_functor{nullptr}, poly_index_map(ordering_functor) {
    }

    void PolynomialIndexStorage::set_factory(const PolynomialFactory &factory) {
        if (!this->poly_index_map.empty()) [[unlikely]] {
            throw std::runtime_error{"Cannot change polynomial factory for non-empty indices."};
        }

        this->factoryPtr = &factory;
        this->ordering_functor.set_factory(factory);
    }


    std::pair<ptrdiff_t, bool> PolynomialIndexStorage::insert(const PolynomialLMIndex &poly, ptrdiff_t offset) {
        assert(this->factoryPtr);
        auto [iter, did_insert] = this->poly_index_map.try_emplace(poly, offset);
        return {iter->second, did_insert};
    }

    ptrdiff_t PolynomialIndexStorage::find(const PolynomialLMIndex& poly) const noexcept {
        assert(this->factoryPtr);
        auto where_iter = this->poly_index_map.find(poly);
        if (where_iter == this->poly_index_map.cend()) {
            return -1;
        }
        return where_iter->second;
    }

    static_assert(Moment::stores_indices<PolynomialIndexStorage, PolynomialLMIndex>);


}