/**
 * symbol_combo_to_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol_combo_to_basis.h"

#include "symbol_table.h"

#include "utilities/float_utils.h"

namespace Moment {

    namespace {
        inline std::map<symbol_name_t, std::pair<double, double>>
        zip_basis(const SymbolTable& symbols, const basis_vec_t& real_basis, const basis_vec_t& img_basis) {
            // zip real & im bits into corresponding symbols
            std::map<symbol_name_t, std::pair<double, double>> zipped;
            for (auto re_basis_iter = basis_vec_t::InnerIterator(real_basis); re_basis_iter; ++re_basis_iter) {
                const auto re_basis_id = static_cast<ptrdiff_t>(re_basis_iter.index());
                assert(re_basis_id >= 0);
                const double re_value = re_basis_iter.value();

                if (re_basis_id >= symbols.Basis.RealSymbolCount()) {
                    throw errors::unknown_basis_elem{true, re_basis_id};
                }
                const size_t symbol_index = symbols.Basis.RealSymbols()[re_basis_id];
                const auto &symbol_info = symbols[symbol_index];
                zipped.emplace(std::make_pair(symbol_info.Id(), std::make_pair(re_value, 0.0)));
            }
            for (auto im_basis_iter = basis_vec_t::InnerIterator(img_basis); im_basis_iter; ++im_basis_iter) {
                const auto im_basis_id = static_cast<ptrdiff_t>(im_basis_iter.index());
                assert(im_basis_id >= 0);
                const double im_value = im_basis_iter.value();

                if (im_basis_id >= symbols.Basis.ImaginarySymbolCount()) {
                    throw errors::unknown_basis_elem{false, im_basis_id};
                }
                const size_t symbol_index = symbols.Basis.ImaginarySymbols()[im_basis_id];
                const auto &symbol_info = symbols[symbol_index];

                auto found_real = zipped.find(symbol_info.Id());
                if (found_real != zipped.end()) {
                    found_real->second.second = im_value;
                } else {
                    zipped.emplace(std::make_pair(symbol_info.Id(), std::make_pair(0.0, im_value)));
                }
            }
            return zipped;
        }
    }

    std::pair<basis_vec_t, basis_vec_t>
    SymbolComboToBasisVec::operator()(const SymbolCombo& combo) const {

        // Prepare sparse vector outputs:
        auto output = std::make_pair(
            basis_vec_t(static_cast<basis_vec_t::Index>(this->symbols.Basis.RealSymbolCount())),
            basis_vec_t(static_cast<basis_vec_t::Index>(this->symbols.Basis.ImaginarySymbolCount()))
        );

        for (auto iter = combo.begin(); iter != combo.end(); ++iter) {
        //for (const auto& expr : combo) {
            const auto& expr = *iter;
            if (expr.id >= this->symbols.size()) {
                throw errors::unknown_symbol{expr.id};
            }
            const auto& symbolInfo = this->symbols[expr.id];
            const auto [re_basis_idx, im_basis_idx] = symbolInfo.basis_key();

            const auto [next_is_cc, cc_factor] = [&]() -> std::pair<bool, double> {
                if (expr.conjugated) {
                    // Assume ordering X, X*; so if this is conjugated, following symbol must differ.
                    return {false, 0.0};
                }
                auto peek_next_iter = iter + 1;
                if (peek_next_iter == combo.end()) {
                    return {false, 0.0};
                }
                if (peek_next_iter->id != expr.id) {
                    return {false, 0.0};
                }
                assert(peek_next_iter->conjugated); // Assume ordering X, X*
                return {true, peek_next_iter->factor};
            }();
            assert(!expr.conjugated || (cc_factor == 0.0));

            const double real_part = expr.factor + cc_factor;
            const double im_part = ((expr.conjugated ? -1.0 : 1.0) * expr.factor) - cc_factor;



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

    SymbolCombo BasisVecToSymbolCombo::operator()(const basis_vec_t& real_basis,
                                                  const basis_vec_t& img_basis) const {
        const auto zipped_basis = zip_basis(this->symbols, real_basis, img_basis);

        SymbolCombo::storage_t output;

        for (const auto [symbol_id, values] : zipped_basis) {
            const auto& symbol_info = this->symbols[symbol_id];
            if (symbol_info.is_hermitian()) {
                assert(approximately_zero(values.second));
                output.emplace_back(symbol_id, values.first, false);

            } else if (symbol_info.is_antihermitian()) {
                assert(approximately_zero(values.first));
                output.emplace_back(symbol_id, values.second, false); // A* = -A;
            } else {
                // Add X
                const double coef = 0.5 * (values.first + values.second);
                if (!approximately_zero(coef)) {
                    output.emplace_back(symbol_id, coef, false);
                }
                // Add X*
                const double conj_coef = 0.5 * (values.first - values.second);
                if (!approximately_zero(conj_coef)) {
                    output.emplace_back(symbol_id, conj_coef, true);
                }
            }
        }
        return SymbolCombo{std::move(output)};
    }
}