/**
 * raw_polynomial.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_sequence.h"

#include "integer_types.h"

#include <complex>
#include <string>
#include <vector>

namespace Moment {
    class Context;
    class SymbolTable;
    class Polynomial;
    class PolynomialFactory;

    /**
      * A 'raw' polynomial consists of an operator sequence and weight.
      * This is necessary for the correct generation of Polynomial localizing matrices where moment aliasing can occurs.
      */
    struct RawPolynomialElement {
        OperatorSequence sequence;
        std::complex<double> weight = 1.0;

        RawPolynomialElement(OperatorSequence seq, std::complex<double> w)
                : sequence{std::move(seq)}, weight{w} {

            // Move sign from sequence to weight
            auto sign = sequence.get_sign();
            switch (sign) {
                case SequenceSignType::Positive:
                    break;
                case SequenceSignType::Imaginary:
                    weight *= std::complex{0.0, 1.0};
                    sequence.set_sign(SequenceSignType::Positive);
                    break;
                case SequenceSignType::Negative:
                    weight *= -1;
                    sequence.set_sign(SequenceSignType::Positive);
                    break;
                case SequenceSignType::NegativeImaginary:
                    weight *= std::complex{0.0, -1.0};
                    sequence.set_sign(SequenceSignType::Positive);
                    break;
            }
        }

        RawPolynomialElement(const RawPolynomialElement& rhs) = delete;

        RawPolynomialElement(RawPolynomialElement&& rhs) = default;
    };

    class RawPolynomial {
    private:
        std::vector<RawPolynomialElement> data;

    public:
        RawPolynomial() = default;
        RawPolynomial(const RawPolynomial& rhs) = delete;
        RawPolynomial(RawPolynomial&& rhs) = default;

        inline void emplace_back(OperatorSequence seq, std::complex<double> w) {
            data.emplace_back(std::move(seq), w);
        }

        inline size_t size() const noexcept {
            return this->data.size();
        }

        inline auto begin() const noexcept {
            return this->data.cbegin();
        }

        inline auto end() const noexcept {
            return this->data.cend();
        }

        [[nodiscard]] std::string to_string(const Context& context) const;

        /** Find symbols for Polynomial, and create appropriate object */
        [[nodiscard]] Polynomial to_polynomial(const PolynomialFactory& factory) const;

        /**
         * Find or register symbols for Polynomial, and create appropriate object
         */
        [[nodiscard]] Polynomial to_polynomial_register_symbols(const PolynomialFactory& factory,
                                                                SymbolTable& symbols) const;

    };
}
