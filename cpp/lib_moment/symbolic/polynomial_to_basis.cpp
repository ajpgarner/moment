/**
 * polynomial_to_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_to_basis.h"

#include "polynomial_factory.h"
#include "symbol_table.h"

#include "utilities/float_utils.h"

namespace Moment {

    namespace {
        // Static asserts: safety, in case types are later changed!
        static_assert(std::is_same_v<Eigen::SparseVector<double>, basis_vec_t>);
        static_assert(std::is_same_v<Eigen::SparseVector<std::complex<double>>, complex_basis_vec_t>);

        template<class number_t>
        std::map<symbol_name_t, std::pair<number_t, number_t>>
        zip_basis(const SymbolTable& symbols, const Eigen::SparseVector<number_t>& real_basis,
                                              const Eigen::SparseVector<number_t>& img_basis) {
            using bv_t = Eigen::SparseVector<number_t>;
            using inner_iter_t = typename bv_t::InnerIterator;

            // zip real & im bits into corresponding symbols
            std::map<symbol_name_t, std::pair<number_t, number_t>> zipped;
            for (auto re_basis_iter = inner_iter_t(real_basis); re_basis_iter; ++re_basis_iter) {
                const auto re_basis_id = static_cast<ptrdiff_t>(re_basis_iter.index());
                assert(re_basis_id >= 0);
                const number_t re_value = re_basis_iter.value();

                if (re_basis_id >= symbols.Basis.RealSymbolCount()) {
                    throw errors::unknown_basis_elem{true, re_basis_id};
                }
                const size_t symbol_index = symbols.Basis.RealSymbols()[re_basis_id];
                const auto &symbol_info = symbols[symbol_index];
                zipped.emplace(std::make_pair(symbol_info.Id(), std::make_pair(re_value, number_t{0})));
            }
            for (auto im_basis_iter = inner_iter_t(img_basis); im_basis_iter; ++im_basis_iter) {
                const auto im_basis_id = static_cast<ptrdiff_t>(im_basis_iter.index());
                assert(im_basis_id >= 0);
                const number_t im_value = im_basis_iter.value();

                if (im_basis_id >= symbols.Basis.ImaginarySymbolCount()) {
                    throw errors::unknown_basis_elem{false, im_basis_id};
                }
                const size_t symbol_index = symbols.Basis.ImaginarySymbols()[im_basis_id];
                const auto &symbol_info = symbols[symbol_index];

                auto found_real = zipped.find(symbol_info.Id());
                if (found_real != zipped.end()) {
                    found_real->second.second = im_value;
                } else {
                    zipped.emplace(std::make_pair(symbol_info.Id(), std::make_pair(number_t{0}, im_value)));
                }
            }
            return zipped;
        }

        template<class number_t>
        Polynomial do_basis_vec_to_symbol_combo(const PolynomialFactory& factory,
                                                const Eigen::SparseVector<number_t>& real_basis,
                                                const Eigen::SparseVector<number_t>& img_basis) {
            const auto& symbols = factory.symbols;
            const auto zipped_basis = zip_basis(symbols, real_basis, img_basis);

            Polynomial::storage_t output;

            for (const auto [symbol_id, values] : zipped_basis) {
                const auto& symbol_info = symbols[symbol_id];
                if (symbol_info.is_hermitian()) {
                    assert(approximately_zero(values.second, factory.zero_tolerance)); // No imaginary part??
                    output.emplace_back(symbol_id, values.first, false);

                } else if (symbol_info.is_antihermitian()) {
                    assert(approximately_zero(values.first, factory.zero_tolerance)); // No real part??
                    output.emplace_back(symbol_id, values.second, false); // A* = -A;
                } else {
                    // Add X
                    const number_t coef = 0.5 * (values.first + values.second);
                    if (!approximately_zero(coef,  factory.zero_tolerance)) {
                        output.emplace_back(symbol_id, coef, false);
                    }
                    // Add X*
                    const number_t conj_coef = 0.5 * (values.first - values.second);
                    if (!approximately_zero(conj_coef, factory.zero_tolerance)) {
                        output.emplace_back(symbol_id, conj_coef, true);
                    }
                }
            }
            return factory(std::move(output));
        }

        template<bool merge_in_im = false>
        void do_polynomial_to_triplets(const SymbolTable& symbols, double zero_tolerance,
                                       const Polynomial& polynomial,
                                       const Eigen::Index basis_Z_real, const Eigen::Index basis_Z_imaginary,
                                       std::vector<Eigen::Triplet<double>>& real_triplets,
                                       std::vector<Eigen::Triplet<double>>& imaginary_triplets,
                                       Eigen::Index im_col_offset) {

            assert(!merge_in_im || (&real_triplets == &imaginary_triplets)); // If merging in only one triplet object.
            const Eigen::Index im_row_offset = merge_in_im ? im_col_offset : 0;

            for (auto iter = polynomial.begin(); iter != polynomial.end(); ++iter) {
                // Get monomial X, check it is valid
                const auto& expr = *iter;
                if (expr.id >= symbols.size()) {
                    throw errors::unknown_symbol{expr.id};
                }
                const auto& symbolInfo = symbols[expr.id];

                // Get real and imaginary basis indices for X
                const auto [basis_X_real, basis_X_imaginary] = symbolInfo.basis_key();

                const auto [next_is_cc, factor, cc_factor] = [&]() -> std::tuple<bool, std::complex<double>, std::complex<double>> {
                    if (expr.conjugated) {
                        // Assume ordering X, X*; so if this is conjugated, following symbol must differ.
                        return {false, 0, expr.factor};
                    }

                    auto peek_next_iter = iter + 1;
                    if (peek_next_iter == polynomial.end()) {
                        return {false, expr.factor, 0};
                    }
                    if (peek_next_iter->id != expr.id) {
                        assert(!expr.conjugated);
                        return {false, expr.factor, 0};
                    }
                    assert(peek_next_iter->conjugated); // Assume ordering X, X*
                    return {true, expr.factor, peek_next_iter->factor};
                }();
                assert(!expr.conjugated || (approximately_zero(cc_factor, zero_tolerance)));

                const double az_factor_ax = factor.real() + cc_factor.real();
                const double az_factor_bx = -factor.imag() + cc_factor.imag();

                const double bz_factor_ax = factor.imag() + cc_factor.imag();
                const double bz_factor_bx = factor.real() - cc_factor.real();

                // Re(X) can be non-zero
                if (basis_X_real >= 0) {
                    // Re(Z) can be non-zero
                    if ((basis_Z_real >= 0) && (!approximately_zero(az_factor_ax, zero_tolerance))) {
                        real_triplets.emplace_back(basis_Z_real, basis_X_real, az_factor_ax);
                    }
                    // Im(Z) can be non-zero
                    if ((basis_Z_imaginary >= 0) && (!approximately_zero(bz_factor_ax, zero_tolerance))) {
                        imaginary_triplets.emplace_back(basis_Z_imaginary + im_row_offset, basis_X_real, bz_factor_ax);
                    }
                }

                // Im(X) can be non-zero
                if (basis_X_imaginary >= 0) {
                    // Re(Z) can be non-zero
                    if ((basis_Z_real >= 0) && (!approximately_zero(az_factor_bx, zero_tolerance))) {
                        real_triplets.emplace_back(basis_Z_real, basis_X_imaginary + im_col_offset, az_factor_bx);
                    }
                    // Im(Z) can be non-zero
                    if ((basis_Z_imaginary >= 0) && (!approximately_zero(bz_factor_bx, zero_tolerance))) {
                        imaginary_triplets.emplace_back(basis_Z_imaginary + im_row_offset,
                                                        basis_X_imaginary + im_col_offset,
                                                        bz_factor_bx);
                    }
                }

                // If next element is CC of this element, skip it (we've already handled CC case!)
                if (next_is_cc) {
                    ++iter;
                }
            }
        }

        template<class number_t>
        inline constexpr number_t get_factor(std::complex<double> number) {
            return number;
        }

        template<>
        inline constexpr double get_factor<double>(std::complex<double> number) {
            return number.real();
        }

        template<class number_t>
        std::pair<Eigen::SparseVector<number_t>, Eigen::SparseVector<number_t>>
        do_polynomial_to_basis_vec(const SymbolTable& symbols, double zero_tolerance, const Polynomial& combo) {

            using basis_t = typename Eigen::SparseVector<number_t>;
            using index_t = typename Eigen::SparseVector<number_t>::Index;

            // Prepare sparse vector outputs:
            auto output = std::make_pair(
                    basis_t(static_cast<index_t>(symbols.Basis.RealSymbolCount())),
                    basis_t(static_cast<index_t>(symbols.Basis.ImaginarySymbolCount()))
            );

            for (auto iter = combo.begin(); iter != combo.end(); ++iter) {
                //for (const auto& expr : combo) {
                const auto& expr = *iter;
                if (expr.id >= symbols.size()) {
                    throw errors::unknown_symbol{expr.id};
                }
                const auto& symbolInfo = symbols[expr.id];
                const auto [re_basis_idx, im_basis_idx] = symbolInfo.basis_key();

                const auto [next_is_cc, cc_factor] = [&]() -> std::pair<bool, number_t> {
                    if (expr.conjugated) {
                        // Assume ordering X, X*; so if this is conjugated, following symbol must differ.
                        return {false, number_t{0}};
                    }
                    auto peek_next_iter = iter + 1;
                    if (peek_next_iter == combo.end()) {
                        return {false, number_t{0}};
                    }
                    if (peek_next_iter->id != expr.id) {
                        return {false, number_t{0}};
                    }
                    assert(peek_next_iter->conjugated); // Assume ordering X, X*
                    return {true, get_factor<number_t>(peek_next_iter->factor)};
                }();
                assert(!expr.conjugated || (cc_factor == 0.0));

                const number_t real_part = get_factor<number_t>(expr.factor) + cc_factor;
                const number_t im_part = ((expr.conjugated ? -1.0 : 1.0) * get_factor<number_t>(expr.factor)) - cc_factor;

                if (re_basis_idx >= 0) {
                    if (!approximately_zero(real_part, zero_tolerance)) {
                        output.first.insert(re_basis_idx) = real_part;
                    }
                }

                if (im_basis_idx >= 0) {
                    if (!approximately_zero(im_part, zero_tolerance)) {
                        output.second.insert(im_basis_idx) = im_part;
                    }
                }

                // If next element is CC of this element, skip it (we've already handled CC case!)
                if (next_is_cc) {
                    ++iter;
                }
            }

            output.first.finalize();
            output.second.finalize();

            return output;
        }
    }

    std::pair<basis_vec_t, basis_vec_t>
    PolynomialToBasisVec::operator()(const Polynomial& poly) const {
        assert(poly.real_factors());
        return do_polynomial_to_basis_vec<double>(this->symbols, this->zero_tolerance, poly);
    }


    void PolynomialToBasisVec::add_triplet_row(const Moment::Polynomial &poly,
                                               const Eigen::Index real_row_index, const Eigen::Index im_row_index,
                                               std::vector<Eigen::Triplet<double>> &real_triplets,
                                               std::vector<Eigen::Triplet<double>> &im_triplets) const {
        do_polynomial_to_triplets<false>(this->symbols, this->zero_tolerance, poly, real_row_index, im_row_index,
                                         real_triplets, im_triplets,
                                         static_cast<Eigen::Index>(this->symbols.Basis.RealSymbolCount()));
    }

    void PolynomialToBasisVec::add_triplet_row(const Moment::Polynomial &poly,
                                               const Eigen::Index real_row_index, const Eigen::Index im_row_index,
                                               std::vector<Eigen::Triplet<double>> &combined_triplets) const {
        do_polynomial_to_triplets<true>(this->symbols, this->zero_tolerance, poly, real_row_index, im_row_index,
                                        combined_triplets, combined_triplets,
                                        static_cast<Eigen::Index>(this->symbols.Basis.RealSymbolCount()));
    }

    std::pair<complex_basis_vec_t, complex_basis_vec_t>
    PolynomialToComplexBasisVec::operator()(const Polynomial& combo) const {
        return do_polynomial_to_basis_vec<std::complex<double>>(this->symbols, this->zero_tolerance, combo);
    }



    Polynomial BasisVecToPolynomial::operator()(const basis_vec_t& real_basis,
                                                const basis_vec_t& img_basis) const {
        return do_basis_vec_to_symbol_combo(this->factory, real_basis, img_basis);
    }

    Polynomial ComplexBasisVecToPolynomial::operator()(const complex_basis_vec_t& real_basis,
                                                       const complex_basis_vec_t& img_basis) const {
        return do_basis_vec_to_symbol_combo(this->factory, real_basis, img_basis);
    }


}