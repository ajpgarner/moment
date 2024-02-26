/**
 * export_symbol_combo.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_polynomial.h"
#include "export_operator_sequence.h"

#include "dictionary/raw_polynomial.h"

#include "scenarios/contextual_os_helper.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_to_basis.h"
#include "symbolic/symbol_table.h"

#include "utilities/utf_conversion.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

#include "eigen/export_eigen_sparse.h"


#include "errors.h"

namespace Moment::mex {



    std::pair<matlab::data::SparseArray<std::complex<double>>,
              matlab::data::SparseArray<std::complex<double>>>
        PolynomialExporter::basis(const Polynomial& combo) const {

        PolynomialToComplexBasisVec exporter{this->symbols, this->zero_tolerance};
        const auto [basis_re, basis_im] = exporter(combo);

        return {
            export_eigen_sparse(this->engine, factory, basis_re),
            export_eigen_sparse(this->engine, factory, basis_im)
        };
    }

    std::pair<matlab::data::SparseArray<std::complex<double>>,
              matlab::data::SparseArray<std::complex<double>>>
    PolynomialExporter::basis(const std::span<const Polynomial> polys) const {
        PolynomialToComplexBasisVec exporter{this->symbols, this->zero_tolerance};

        const auto [real, imaginary] = exporter(polys);

        return {
            export_eigen_sparse(this->engine, factory, real),
            export_eigen_sparse(this->engine, factory, imaginary)
        };
    }

    matlab::data::CellArray PolynomialExporter::symbol_cell(const Polynomial& polynomial) const {

        auto output = factory.createCellArray({1, polynomial.size()});
        auto write_iter = output.begin();
        for (const auto& term : polynomial) {
            auto symbol_expr_out = factory.createCellArray({1ULL, term.conjugated ? 3ULL : 2ULL});
            symbol_expr_out[0] = factory.createScalar<int64_t>(term.id);
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

    matlab::data::CellArray PolynomialExporter::symbol_cell(const Monomial& monomial) const {

        auto output = factory.createCellArray({1, 1});
        auto write_iter = output.begin();

        auto symbol_expr_out = factory.createCellArray({1ULL, monomial.conjugated ? 3ULL : 2ULL});
        symbol_expr_out[0] = factory.createScalar<int64_t>(monomial.id);
        if (approximately_real(monomial.factor)) {
            symbol_expr_out[1] = factory.createScalar<double>(monomial.factor.real());
        } else {
            symbol_expr_out[1] = factory.createScalar<std::complex<double>>(monomial.factor);
        }
        if (monomial.conjugated) {
            symbol_expr_out[2] = factory.createScalar<bool>(true);
        }
        (*output.begin()) = std::move(symbol_expr_out);
        return output;
    }


    matlab::data::MATLABString PolynomialExporter::string(const Polynomial &poly, const bool show_braces) const {
        StringFormatContext sfc{this->context, this->symbols};
        sfc.format_info.show_braces = show_braces;
        sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

        return {UTF8toUTF16Convertor::convert(
                make_contextualized_string(sfc, [&poly](ContextualOS& os) { os << poly; })
        )};
    }


    FullMonomialSpecification
    PolynomialExporter::sequences(const Moment::RawPolynomial& raw_polynomial) const {
        FullMonomialSpecification output{factory, raw_polynomial.size(), false, false}; // no symbols, no aliases

        auto op_iter = output.operators.begin();
        auto coef_iter = output.coefficients.begin();
        auto hash_iter = output.hashes.begin();

        for (const auto& [op_seq, weight]: raw_polynomial) {
            *op_iter = export_operator_sequence(factory, op_seq, true);
            *coef_iter = weight;
            *hash_iter = op_seq.hash();

            // Advance output iterators
            ++op_iter;
            ++coef_iter;
            ++hash_iter;
        }

        return output;
    }


    FullMonomialSpecification
    PolynomialExporter::sequences(const Moment::Polynomial& polynomial, bool include_symbols) const {

        const bool include_aliases = this->symbols.can_have_aliases && include_symbols;

        FullMonomialSpecification output{factory, polynomial.size(), include_symbols, include_aliases};

        auto op_iter = output.operators.begin();
        auto coef_iter = output.coefficients.begin();
        auto hash_iter = output.hashes.begin();
        auto symbol_iter = include_symbols ? output.symbol_ids.begin() : output.symbol_ids.end();
        auto conj_iter = output.is_conjugated.begin();
        auto real_basis_iter = include_symbols ? output.real_basis_elems.begin() : output.real_basis_elems.end();
        auto im_basis_iter = include_symbols ? output.im_basis_elems.begin() : output.im_basis_elems.end();
        auto alias_iter = include_aliases ? output.is_aliased.begin() : output.is_aliased.end();

        size_t idx = 0;
        for (const auto& term : polynomial) {
            // Check symbol ID is good
            if ((term.id < 0) || (term.id >= symbols.size())) {
                std::stringstream errSS;
                errSS << "Could not resolve symbol '" << term.id << "' at index " << (idx+1) << ".";
                throw InternalError{errSS.str()};
            }
            const auto& symbol_info = symbols[term.id];

            // Check we have a sequence
            if (!symbol_info.has_sequence()) {
                std::stringstream errSS;
                errSS << "Symbol '" << term.id << "' at index " << (idx+1) << " is not associated with an operator sequence.";
                throw InternalError{errSS.str()};
            }
            const auto& op_seq = term.conjugated ? symbol_info.sequence_conj() : symbol_info.sequence();

            // Write outputs
            *op_iter = export_operator_sequence(factory, op_seq, true);
            *coef_iter = term.factor;
            *hash_iter = op_seq.hash();
            if (include_symbols) {
                *symbol_iter = term.id;
                *conj_iter = term.conjugated;
                *real_basis_iter = symbol_info.basis_key().first + 1; // ML indexing
                *im_basis_iter = symbol_info.basis_key().second + 1; // ML indexing
                if (include_aliases) {
                    *alias_iter = false;
                }
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
                if (include_aliases) {
                    ++alias_iter;
                }
            }
        }
        return output;
    }

    matlab::data::CellArray PolynomialExporter::symbol_cell_vector(const std::span<const Polynomial> poly_list,
                                                                   matlab::data::ArrayDimensions shape) const {
        auto output = factory.createCellArray(std::move(shape));

        auto write_iter = output.begin();
        for (const auto& poly : poly_list) {
            (*write_iter) = this->symbol_cell(poly);
            ++write_iter;
        }

        return output;
    }
    matlab::data::CellArray PolynomialExporter::symbol_cell_vector(const std::span<const Monomial> mono_list,
                                                                   matlab::data::ArrayDimensions shape) const {
        auto output = factory.createCellArray(std::move(shape));

        auto write_iter = output.begin();
        for (const auto& mono : mono_list) {
            (*write_iter) = this->symbol_cell(mono);
            ++write_iter;
        }

        return output;
    }

    matlab::data::CellArray
    PolynomialExporter::sequence_cell_vector(std::span<const Polynomial> poly_list,
                                             const std::vector<size_t>& shape,
                                             bool include_symbols) const {
        auto output = factory.createCellArray(shape);

        auto write_iter = output.begin();
        for (const auto& poly : poly_list) {
            auto poly_spec = this->sequences(poly, include_symbols);
            (*write_iter) = poly_spec.move_to_cell(factory);
            ++write_iter;
        }

        return output;
    }

    matlab::data::CellArray
    PolynomialExporter::sequence_cell_vector(std::span<const RawPolynomial> poly_list,
                                             const std::vector<size_t>& shape) const {
        auto output = factory.createCellArray(shape);

        auto write_iter = output.begin();
        for (const auto& poly : poly_list) {
            auto poly_spec = this->sequences(poly);
            (*write_iter) = poly_spec.move_to_cell(factory);
            ++write_iter;
        }

        return output;
    }

    FullMonomialSpecification
    PolynomialExporter::monomial_sequence_cell_vector(std::span<const Polynomial> poly_list,
                                                      const std::vector<size_t>& shape,
                                                      bool include_symbols) const {
        FullMonomialSpecification fms{this->factory, shape, include_symbols};

        if (include_symbols) {
            auto write_iter = fms.full_write_begin();
            FullMonomialSpecification::FullWriteFunctor functor{this->factory, this->symbols};
            for (const auto& poly : poly_list) {
                assert(poly.is_monomial());
                if (poly.empty()) {
                    *write_iter = functor(Monomial{0, 0.0});
                } else {
                    *write_iter = functor(poly.back());
                }
                ++write_iter;
            }
        } else {
            auto write_iter = fms.partial_write_begin();
            FullMonomialSpecification::PartialWriteFunctor functor{this->factory, this->symbols};
            for (const auto& poly : poly_list) {
                assert(poly.is_monomial());
                if (poly.empty()) {
                    *write_iter = functor(Monomial{0, 0.0});
                } else {
                    *write_iter = functor(poly.back());
                }
                ++write_iter;
            }
        }

        return fms;
    }

    FullMonomialSpecification
    PolynomialExporter::monomial_sequence_cell_vector(std::span<const RawPolynomial> poly_list,
                                                      const std::vector<size_t>& shape) const {
        FullMonomialSpecification fms{this->factory, shape, false};

        auto write_iter = fms.partial_write_begin();
        FullMonomialSpecification::PartialWriteFunctor functor{this->factory, this->symbols};
        for (const auto& poly : poly_list) {
            assert(poly.size() <= 1);
            if (poly.empty()) {
                *write_iter = functor(Monomial{0, 0.0});
            } else {
                *write_iter = functor(poly[0].sequence, poly[0].weight);
            }
            ++write_iter;
        }


        return fms;
    }

}
