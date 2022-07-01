/**
 * pm_indices.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <initializer_list>
#include <span>
#include <vector>

namespace NPATK {

    template<typename type_t>
    class RecursiveStorage {

    protected:
        type_t object;
    private:
        ptrdiff_t index_offset = 0;
        std::vector<RecursiveStorage<type_t>> subindices{ };

    public:
        constexpr RecursiveStorage(size_t width, size_t max_depth, const type_t& zero)
            : object{zero} {
            this->subindices.reserve(width);
            if (max_depth > 0) {
                for (size_t i = 0; i < width; ++i) {
                    this->subindices.emplace_back(width, max_depth - 1, zero);
                }
            }
        }

        constexpr void set(type_t&& the_object) noexcept {
            this->object = std::move(the_object);
        }

        constexpr void set(const type_t& the_object) noexcept {
            this->object = the_object;
        }

        constexpr void set(std::span<const size_t> indices, const type_t& the_object) noexcept {
            if (indices.empty()) {
                this->object = the_object;
                return;
            }

            auto front = static_cast<ptrdiff_t>(indices.front());
            assert(front >= -this->index_offset);
            assert(front < subindices.size());
            this->subindices[front + this->index_offset].set(indices.last(indices.size() - 1), the_object);
        }

        constexpr void set(std::initializer_list<size_t> indices, const type_t& the_object) noexcept {
            std::vector<size_t> v(indices);
            set(v, the_object);
        }

        [[nodiscard]] constexpr const type_t& access() const noexcept {
            return object;
        }

        [[nodiscard]] constexpr const type_t& access(std::span<const size_t> indices) const noexcept {
            if (indices.empty()) {
                return access();
            }

            auto front = static_cast<ptrdiff_t>(indices.front());
            assert(front >= -this->index_offset);
            assert(front < subindices.size());
            return this->subindices[front].access(indices.last(indices.size() - 1));
        }

        [[nodiscard]] constexpr const type_t& access(std::initializer_list<size_t> indices) const noexcept {
            std::vector<size_t> v(indices);
            return access(v);
        }
    };

    class RecursiveIndex : public RecursiveStorage<ptrdiff_t> {
    public:
        constexpr RecursiveIndex(size_t width, size_t max_depth)
             : RecursiveStorage<ptrdiff_t>{width, max_depth, -1} { }

        [[nodiscard]] constexpr bool is_set() const noexcept { return this->object >= 0; }
    };

    class RecursiveDoubleIndex : public RecursiveStorage<std::pair<ptrdiff_t, ptrdiff_t>> {
    public:
        constexpr RecursiveDoubleIndex(size_t width, size_t max_depth)
             : RecursiveStorage<std::pair<ptrdiff_t, ptrdiff_t>>{width, max_depth, {-1, 0}} { }

    };

}