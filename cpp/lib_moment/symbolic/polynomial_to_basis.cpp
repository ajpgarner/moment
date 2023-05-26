/**
 * polynomial_to_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_to_basis.h"

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
        Polynomial do_basis_vec_to_symbol_combo(const SymbolTable& symbols,
                                                const Eigen::SparseVector<number_t>& real_basis,
                                                const Eigen::SparseVector<number_t>& img_basis) {
            const auto zipped_basis = zip_basis(symbols, real_basis, img_basis);

            Polynomial::storage_t output;

            for (const auto [symbol_id, values] : zipped_basis) {
                const auto& symbol_info = symbols[symbol_id];
                if (symbol_info.is_hermitian()) {
                    assert(approximately_zero(values.second)); // No imaginary part??
                    output.emplace_back(symbol_id, values.first, false);

                } else if (symbol_info.is_antihermitian()) {
                    assert(approximately_zero(values.first)); // No real part??
                    output.emplace_back(symbol_id, values.second, false); // A* = -A;
                } else {
                    // Add X
                    const number_t coef = 0.5 * (values.first + values.second);
                    if (!approximately_zero(coef)) {
                        output.emplace_back(symbol_id, coef, false);
                    }
                    // Add X*
                    const number_t conj_coef = 0.5 * (values.first - values.second);
                    if (!approximately_zero(conj_coef)) {
                        output.emplace_back(symbol_id, conj_coef, true);
                    }
                }
            }
            return Polynomial{std::move(output)};
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
        do_symbol_combo_to_basis_vec(const SymbolTable& symbols, const Polynomial& combo) {

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
                    if (!approximately_zero(real_part)) {
                        output.first.insert(re_basis_idx) = real_part;
                    }
                }

                if (im_basis_idx >= 0) {
                    if (!approximately_zero(im_part)) {
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
    PolynomialToBasisVec::operator()(const Polynomial& combo) const {
        assert(combo.real_factors());
        return do_symbol_combo_to_basis_vec<double>(this->symbols, combo);
    }

    std::pair<complex_basis_vec_t, complex_basis_vec_t>
    PolynomialToComplexBasisVec::operator()(const Polynomial& combo) const {
        return do_symbol_combo_to_basis_vec<std::complex<double>>(this->symbols, combo);
    }

    Polynomial BasisVecToPolynomial::operator()(const basis_vec_t& real_basis,
                                                const basis_vec_t& img_basis) const {
        return do_basis_vec_to_symbol_combo(this->symbols, real_basis, img_basis);
    }

    Polynomial ComplexBasisVecToPolynomial::operator()(const complex_basis_vec_t& real_basis,
                                                       const complex_basis_vec_t& img_basis) const {
        return do_basis_vec_to_symbol_combo(this->symbols, real_basis, img_basis);
    }
}