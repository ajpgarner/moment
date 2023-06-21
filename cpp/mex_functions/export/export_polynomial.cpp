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


    matlab::data::CellArray PolynomialExporter::sequences(const Moment::Polynomial &combo) const {
        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({1, combo.size()});
        auto write_iter = output.begin();
        size_t idx = 0;
        for (const auto& term : combo) {
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

            auto symbol_expr_out = factory.createCellArray({1ULL, 2ULL});
            symbol_expr_out[0] = export_operator_sequence(factory, op_seq, true);
            if (approximately_real(term.factor, this->zero_tolerance)) {
                symbol_expr_out[1] = factory.createScalar<double>(term.factor.real());
            } else {
                symbol_expr_out[1] = factory.createScalar<std::complex<double>>(term.factor);
            }

            *write_iter = std::move(symbol_expr_out);
            ++write_iter;
        }
        return output;
    }


}
