/**
 * polynomial_localizing_matrix_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "localizing_matrix_index.h"
#include "symbolic/polynomial.h"

#include <cassert>
#include <complex>

namespace Moment {
    class SymbolTable;

    struct PolynomialLMIndex {
    public:
        size_t Level;
        class Polynomial Polynomial;

    public:
        class MonomialLMIterator {
        public:
            using internal_iter_t = decltype(Polynomial.begin());
            using value_type = LocalizingMatrixIndex;

        private:
            const SymbolTable* symbolPtr;
            internal_iter_t iter;
            size_t level;
        public:
            explicit MonomialLMIterator(const SymbolTable& symbols, size_t level,  internal_iter_t iter) noexcept
                : symbolPtr{&symbols}, iter{iter}, level{level} { }

            [[nodiscard]] std::pair<LocalizingMatrixIndex, std::complex<double>> operator*() const noexcept;

            [[nodiscard]] std::complex<double> factor() const noexcept {
                return iter->factor;
            }

            auto& operator++() noexcept {
                ++iter;
                return *this;
            }

            [[nodiscard]] bool operator==(const MonomialLMIterator& other) const noexcept {
                assert(symbolPtr == other.symbolPtr && level == other.level);
                return iter == other.iter;
            }

            [[nodiscard]] bool operator!=(const MonomialLMIterator& other) const noexcept {
                assert(symbolPtr == other.symbolPtr && level == other.level);
                return iter != other.iter;
            }
        };

        class MLMRange {
        private:
            const SymbolTable& symbols;
            size_t level;
            const class Polynomial& polynomial;

        public:
            MLMRange(const SymbolTable& symbols, size_t level, const class Polynomial& poly) noexcept
                        : symbols{symbols}, level{level}, polynomial{poly} { }

            MLMRange(const SymbolTable& symbols, size_t level, class Polynomial&& poly) = delete;

            [[nodiscard]] inline auto begin() const noexcept {
                return MonomialLMIterator{this->symbols, this->level, this->polynomial.begin()};
            }

            [[nodiscard]] inline auto end() const noexcept {
                return MonomialLMIterator{this->symbols, this->level, this->polynomial.end()};
            }
        };

        [[nodiscard]] MLMRange MonomialIndices(const SymbolTable& symbols) const noexcept {
            return MLMRange{symbols, this->Level, this->Polynomial};
        }



    };
}
