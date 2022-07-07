/**
 * recursive_index.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <initializer_list>
#include <span>
#include <utility>
#include <vector>

namespace NPATK {

    template<typename type_t, typename subclass_t>
    class RecursiveStorage {
    protected:
        type_t object;
        ptrdiff_t index_offset = 0;
        std::vector<subclass_t> subindices{};

    public:
        explicit constexpr RecursiveStorage(type_t zero, const ptrdiff_t offset = 0)
            : object(std::move(zero)), index_offset{offset} { }

        [[nodiscard]] size_t num_children() const noexcept { return this->subindices.size(); }

        [[nodiscard]] constexpr subclass_t& subtree(std::span<const size_t> indices) noexcept {
            if (indices.empty()) {
                return static_cast<subclass_t&>(*this);
            }

            auto front = static_cast<ptrdiff_t>(indices.front());
            assert(front >= -this->index_offset);
            assert((front + this->index_offset) < subindices.size());
            return this->subindices[front + this->index_offset].subtree(indices.last(indices.size() - 1));
        }

        [[nodiscard]] constexpr const subclass_t& subtree(std::span<const size_t> indices) const noexcept {
            if (indices.empty()) {
                return static_cast<const subclass_t&>(*this);
            }

            auto front = static_cast<ptrdiff_t>(indices.front());
            assert(front >= -this->index_offset);
            assert((front + this->index_offset) < subindices.size());
            return this->subindices[front + this->index_offset].subtree(indices.last(indices.size() - 1));
        }

        constexpr void set(type_t the_object) noexcept {
            this->object = std::move(the_object);
        }

        constexpr void set(std::span<const size_t> indices, type_t the_object) noexcept {
            auto& place = subtree(indices);
            place.object = std::move(the_object);
        }

        constexpr void set(std::initializer_list<size_t> indices, type_t the_object) noexcept {
            std::vector<size_t> v(indices);
            set(v, std::move(the_object));
        }

        [[nodiscard]] constexpr const type_t& access() const noexcept {
            return object;
        }

        [[nodiscard]] constexpr const type_t& access(std::span<const size_t> indices) const noexcept {
            auto place = subtree(indices);
            return place.access();
        }

        [[nodiscard]] constexpr const type_t& access(std::initializer_list<size_t> indices) const noexcept {
            std::vector<size_t> v(indices);
            return access(v);
        }

        /** Recursively visit each entry in the table */
        template<typename functor_t, typename... Args>
        void visit(functor_t& visitor, Args... args) {
            std::vector<size_t> index_stack;
            do_visit(visitor, index_stack, args...);
        }

        /** Recursively visit each entry in the table */
        template<typename functor_t, typename... Args>
        void visit(functor_t& visitor, Args... args) const {
            std::vector<size_t> index_stack;
            do_visit(visitor, index_stack, args...);
        }

    private:
        template<typename functor_t, typename... Args>
        void do_visit(functor_t& visitor, std::vector<size_t>& indices, Args... args) {
            const auto& const_indices = indices;
            visitor(this->object, const_indices, args...);
            for (size_t cIndex = 0; cIndex < this->num_children(); ++cIndex) {
                indices.push_back(cIndex-this->index_offset);
                this->subindices[cIndex].do_visit(visitor, indices, args...);
            }
            if (!indices.empty()) {
                indices.pop_back();
            }
        }

        template<typename functor_t, typename... Args>
        void do_visit(functor_t& visitor, std::vector<size_t>& indices, Args... args) const {
            const auto& const_indices = indices;
            const auto& this_obj = static_cast<const type_t&>(this->object);
            visitor(this_obj, const_indices, args...);
            for (size_t cIndex = 0; cIndex < this->num_children(); ++cIndex) {
                indices.push_back(cIndex-this->index_offset);
                this->subindices[cIndex].do_visit(visitor, indices, args...);
            }
            if (!indices.empty()) {
                indices.pop_back();
            }
        }

    };

    template<typename type_t, typename subclass_t>
    class WidthByDepthRecursiveStorage : public RecursiveStorage<type_t, subclass_t> {
    protected:
        constexpr WidthByDepthRecursiveStorage(const size_t width, const size_t max_depth, type_t zero)
                : RecursiveStorage<type_t,  subclass_t>(zero) {
            this->subindices.reserve(width);
            if (max_depth > 0) {
                for (size_t i = 0; i < width; ++i) {
                    this->subindices.emplace_back(width, max_depth - 1, zero);
                }
            }
        }

        explicit constexpr WidthByDepthRecursiveStorage(type_t zero)
                : RecursiveStorage<type_t, subclass_t>(std::move(zero)) { }
    };

    template<typename type_t, typename subclass_t>
    class MonotonicChunkRecursiveStorage : public RecursiveStorage<type_t, subclass_t> {
    protected:
        constexpr MonotonicChunkRecursiveStorage(std::span<const size_t> chunk_sizes, size_t max_depth,
                                                 type_t zero, ptrdiff_t offset = 0)
            : RecursiveStorage<type_t, subclass_t>(zero, offset) {

            // Hardcode depth limit
            if (0 == max_depth) {
                return;
            }

            ptrdiff_t next_offset = this->index_offset;
            for (ptrdiff_t i = 0; i < chunk_sizes.size(); ++i) {
                next_offset -= static_cast<ptrdiff_t>(chunk_sizes[i]);

                auto nextChunkSpan = std::span<const size_t>(chunk_sizes.begin() + (i + 1), chunk_sizes.end());

                for (size_t c = 0; c < chunk_sizes[i]; ++c) {
                    if (!nextChunkSpan.empty()) {
                        this->subindices.emplace_back(nextChunkSpan, max_depth-1, zero, next_offset);
                    } else {
                        this->subindices.emplace_back(zero, next_offset);
                    }
                }

            }
        }

        explicit constexpr MonotonicChunkRecursiveStorage(type_t zero, ptrdiff_t offset = 0)
            : RecursiveStorage<type_t, subclass_t>(std::move(zero), offset) { }

    };

    class RecursiveDoubleIndex : public WidthByDepthRecursiveStorage<std::pair<ptrdiff_t, ptrdiff_t>,
                                                                    RecursiveDoubleIndex> {
    public:
        constexpr RecursiveDoubleIndex(size_t width, size_t max_depth, std::pair<ptrdiff_t, ptrdiff_t> zero = {-1, 0})
             : WidthByDepthRecursiveStorage{width, max_depth, zero} { }

        constexpr RecursiveDoubleIndex()
             : WidthByDepthRecursiveStorage{{-1, 0}} { }
    };
}