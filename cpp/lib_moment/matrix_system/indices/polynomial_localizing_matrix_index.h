/**
 * polynomial_localizing_matrix_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "localizing_matrix_index.h"
#include "symbolic/symbol_table.h"
#include "symbolic/polynomial.h"

#include <cassert>
#include <complex>
#include <iosfwd>
#include <string>


namespace Moment {
    class RawPolynomial;
    class SymbolTable;

    /**
     * Generic index for matrices formed by pairing an OSG index with a Polynomial.
     */
    template<typename base_index_t = size_t,
            typename element_index_t = LocalizingMatrixIndex>
    struct PolynomialIndexBase {
    public:
        using OSGIndex = base_index_t;
        using ComponentIndex = element_index_t;

        OSGIndex Level;

        class Polynomial Polynomial;

    public:
        class MonomialLMIterator {
        public:
            using internal_iter_t = decltype(Polynomial.begin());
            using value_type = std::pair<element_index_t, std::complex<double>>;

        private:
            const SymbolTable* symbolPtr;
            internal_iter_t iter;
            OSGIndex level;
        public:
            explicit MonomialLMIterator(const SymbolTable& symbols, OSGIndex level, internal_iter_t iter) noexcept
                : symbolPtr{&symbols}, iter{iter}, level{level} { }

            [[nodiscard]] value_type operator*() const noexcept {
                assert(symbolPtr);
                const auto& monomial = *this->iter;
                assert(monomial.id >= 0 && monomial.id < symbolPtr->size());
                const auto& symbolInfo = (*this->symbolPtr)[monomial.id];
                const auto& opSeqRef = monomial.conjugated ? symbolInfo.sequence_conj() : symbolInfo.sequence();
                return {element_index_t(this->level, opSeqRef), monomial.factor};
            }

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
            OSGIndex level;
            const class Polynomial& polynomial;

        public:
            MLMRange(const SymbolTable& symbols, OSGIndex level, const class Polynomial& poly) noexcept
                        : symbols{symbols}, level{level}, polynomial{poly} { }

            MLMRange(const SymbolTable& symbols, OSGIndex level, class Polynomial&& poly) = delete;

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

    /**
     * Index for a polynomial localizing matrix.
     */
    struct PolynomialLocalizingMatrixIndex : public PolynomialIndexBase<size_t, LocalizingMatrixIndex> {

    public:
        friend std::ostream& operator<<(std::ostream& os, const PolynomialLocalizingMatrixIndex& plmi);

        [[nodiscard]] std::string to_string(const Context& context, const SymbolTable& symbols) const;



        [[nodiscard]] std::string to_string(const MatrixSystem& matrix_system) const;

        [[nodiscard]] static std::string raw_to_string(const Context& context, const SymbolTable& symbols,
                                                   size_t level, const RawPolynomial& raw);
    };


}
