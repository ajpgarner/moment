/**
 * export_symbol_combo.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_polynomial.h"
#include "export_operator_sequence.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_to_basis.h"
#include "symbolic/symbol_table.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

#include "eigen/export_eigen_sparse.h"

#include "error_codes.h"

namespace Moment::mex {

    FullPolynomialSpecification::FullPolynomialSpecification(matlab::data::ArrayFactory &factory,
                                                             size_t length,
                                                             bool include_symbol_info)
        : dimensions({1, length}), has_symbol_info{include_symbol_info},
          operators{factory.createCellArray(dimensions)},
          coefficients{factory.createArray<std::complex<double>>(dimensions)},
          hashes{factory.createArray<uint64_t>(dimensions)},
          symbol_ids{has_symbol_info ? factory.createArray<uint64_t>(dimensions)
                                     : factory.createArray<uint64_t>({0, 0})},
          is_conjugated{has_symbol_info ? factory.createArray<bool>(dimensions)
                                        : factory.createArray<bool>({0, 0})},
          real_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                           : factory.createArray<int64_t>({0, 0})},
          im_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                         : factory.createArray<int64_t>({0, 0})} {

    }

    void FullPolynomialSpecification::move_to_output(IOArgumentRange &output) noexcept {
        switch (output.size()) {
            default:
            case 7:
                output[6] = std::move(this->im_basis_elems);
            case 6:
                output[5] = std::move(this->real_basis_elems);
            case 5:
                output[4] = std::move(this->is_conjugated);
            case 4:
                output[3] = std::move(this->symbol_ids);
            case 3:
                output[2] = std::move(this->hashes);
            case 2:
                output[1] = std::move(this->coefficients);
            case 1:
                output[0] = std::move(this->operators);
            case 0:
                break;
        }
    }

    matlab::data::CellArray FullPolynomialSpecification::move_to_cell(matlab::data::ArrayFactory &factory) {
        auto output = factory.createCellArray({1ULL, this->has_symbol_info ? 7ULL : 3ULL});
        output[0] = std::move(this->operators);
        output[1] = std::move(this->coefficients);
        output[2] = std::move(this->hashes);
        if (this->has_symbol_info) {
            output[3] = std::move(this->symbol_ids);
            output[4] = std::move(this->is_conjugated);
            output[5] = std::move(this->real_basis_elems);
            output[6] = std::move(this->im_basis_elems);
        }
        return output;
    }

    std::pair<matlab::data::SparseArray<std::complex<double>>,
              matlab::data::SparseArray<std::complex<double>>>
        PolynomialExporter::basis(const Polynomial& combo) const {

        PolynomialToComplexBasisVec exporter{this->symbols, this->zero_tolerance};
        auto [basis_re, basis_im] = exporter(combo);

        matlab::data::ArrayFactory factory;
        return {
            export_eigen_sparse(this->engine, factory, basis_re),
            export_eigen_sparse(this->engine, factory, basis_im)
        };
    }

    matlab::data::CellArray PolynomialExporter::direct(const Polynomial &combo) const {

        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({1, combo.size()});
        auto write_iter = output.begin();
        for (const auto& term : combo) {
            auto symbol_expr_out = factory.createCellArray({1ULL, term.conjugated ? 3ULL : 2ULL});
            symbol_expr_out[0] = factory.createScalar<uint64_t>(term.id);
            if (approximately_real(term.factor)) {
                symbol_expr_out[1] = factory.createScalar<double>(term.factor.real());
            } else {
                symbol_expr_out[1] = factory.createScalar<std::complex<double>>(term.factor);
            }
            if (term.conjugated) {
                symbol_expr_out[2] = factory.createScalar<bool>(true);
            }
            *write_iter = std::move( symbol_expr_out);
            ++write_iter;
        }
        return output;
    }


    FullPolynomialSpecification
    PolynomialExporter::sequences(matlab::data::ArrayFactory& factory,
                                  const Moment::Polynomial& polynomial, bool include_symbols) const {

        FullPolynomialSpecification output{factory, polynomial.size(), include_symbols};

        auto op_iter = output.operators.begin();
        auto coef_iter = output.coefficients.begin();
        auto hash_iter = output.hashes.begin();
        auto conj_iter = output.is_conjugated.begin();
        auto symbol_iter = include_symbols ? output.symbol_ids.begin() : output.symbol_ids.end();
        auto real_basis_iter = include_symbols ? output.real_basis_elems.begin() : output.real_basis_elems.end();
        auto im_basis_iter = include_symbols ? output.im_basis_elems.begin() : output.im_basis_elems.end();

        size_t idx = 0;
        for (const auto& term : polynomial) {
            // Check symbol ID is good
            if ((term.id < 0) || (term.id >= symbols.size())) {
                std::stringstream errSS;
                errSS << "Could not resolve symbol '" << term.id << "' at index " << (idx+1) << ".";
                throw_error(this->engine, errors::internal_error, errSS.str());
            }
            const auto& symbol_info = symbols[term.id];

            // Check we have a sequence
            if (!symbol_info.has_sequence()) {
                std::stringstream errSS;
                errSS << "Symbol '" << term.id << "' at index " << (idx+1) << " is not associated with an operator sequence.";
                throw_error(this->engine, errors::internal_error, errSS.str());
            }
            const auto& op_seq = term.conjugated ? symbol_info.sequence_conj() : symbol_info.sequence();

            // Write outputs
            *op_iter = export_operator_sequence(factory, op_seq, true);
            *coef_iter = term.factor;
            *hash_iter = op_seq.hash();
            if (include_symbols) {
                *symbol_iter = term.id;
                *conj_iter = term.conjugated;
                *real_basis_iter = symbol_info.basis_key().first + 1; // ML indexing ???
                *im_basis_iter = symbol_info.basis_key().second + 1; // ML indexing ???
            }

            // Advance output iterators
            ++op_iter;
            ++coef_iter;
            ++hash_iter;
            if (include_symbols) {
                ++symbol_iter;
                ++conj_iter;
                ++real_basis_iter;
                ++im_basis_iter;
            }
        }
        return output;
    }


}
