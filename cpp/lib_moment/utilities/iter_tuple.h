/**
 * iter_tuple.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <concepts>
#include <tuple>
#include <stdexcept>

namespace Moment {

    /**
     * Bunch together iterators representing parallel ranges.
     * Iterator comparison is only done on first iterator!
     */
    template<typename... Iterators>
    struct IterTuple {
        using value_type = std::tuple<typename Iterators::value_type...>;
        using reference = std::tuple<typename Iterators::reference...>;

        std::tuple<Iterators...> iters;

        explicit IterTuple(Iterators&&... argIters)
            : iters{std::forward<Iterators>(argIters)...} {
            static_assert(std::tuple_size_v<decltype(this->iters)> != 0);
        }

        IterTuple& operator++() {
            std::apply([](Iterators&... i) { ((++i), ...); }, this->iters);
            return *this;
        }

        [[nodiscard]] reference operator*() const noexcept {
            return std::apply([](const Iterators&... i) -> reference {
                return reference((*i) ...);
            }, this->iters);
        }

        [[nodiscard]] bool operator==(const IterTuple<Iterators...>& rhs) const noexcept {
            return std::get<0>(this->iters) == std::get<0>(rhs.iters);;
        }

        [[nodiscard]] bool operator!=(const IterTuple<Iterators...>& rhs) const noexcept {
            return std::get<0>(this->iters) != std::get<0>(rhs.iters);;
        }
    };


}
