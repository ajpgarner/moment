/**
 * virtual_polynomial_view.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence.h"
#include "symbolic/polynomial.h"

#include <complex>

#include <cassert>

namespace Moment {
    class CollinsGisin;

    /**
     * Translates a CG iter to a series of operator sequence / coefficient pairs.
     */
    class VirtualPolynomialView {
    private:
        const CollinsGisin& collins_gisin;
        const Polynomial& input_poly;

    public:
        class VPVIter {
            friend class VirtualPolynomialView;
        private:
            const VirtualPolynomialView& view;
            Polynomial::storage_t::const_iterator poly_iter;

        public:
            VPVIter(const VPVIter& rhs) noexcept = default;
            VPVIter(VPVIter&& rhs) noexcept = default;

        private:
            explicit VPVIter(const VirtualPolynomialView& view)
                : view{view}, poly_iter{view.input_poly.begin()} { }

            explicit VPVIter(const VirtualPolynomialView& view, bool end)
                    : view{view}, poly_iter{view.input_poly.end()} { }
        public:

            inline auto& operator++() {
                ++poly_iter;
                return *this;
            }

            [[nodiscard]] std::pair<OperatorSequence, std::complex<double>> operator*() const;

            [[nodiscard]] bool operator==(const VPVIter& rhs) const {
                assert(&this->view == &rhs.view);
                return this->poly_iter == rhs.poly_iter;
            }

            [[nodiscard]] bool operator!=(const VPVIter& rhs) const {
                assert(&this->view == &rhs.view);
                return this->poly_iter != rhs.poly_iter;
            }
        };

    public:
        VirtualPolynomialView(const CollinsGisin& cgTensor, const Polynomial& input_poly)
            : collins_gisin{cgTensor}, input_poly{input_poly} { }

        VirtualPolynomialView(const CollinsGisin& cgTensor, Polynomial&& input_poly) = delete;

        [[nodiscard]] inline VPVIter begin() const {
            return VPVIter{*this};
        }

        [[nodiscard]] inline VPVIter end() const {
            return VPVIter{*this, true};
        }

        [[nodiscard]] inline size_t size() const noexcept { return this->input_poly.size(); }

        [[nodiscard]] inline bool empty() const noexcept { return this->input_poly.empty(); }
    };
}