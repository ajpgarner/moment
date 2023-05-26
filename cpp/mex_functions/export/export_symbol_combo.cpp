/**
 * export_symbol_combo.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_symbol_combo.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_to_basis.h"
#include "symbolic/symbol_table.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

#include "eigen/export_eigen_sparse.h"

#include "error_codes.h"

namespace Moment::mex {

    std::pair<matlab::data::Array, matlab::data::Array>
        SymbolComboExporter::operator()(const Polynomial& combo) const {

        auto [basis_re, basis_im] = PolynomialToBasisVec{symbols}(combo);

        matlab::data::ArrayFactory factory;

        return {
            export_eigen_sparse(this->engine, factory, basis_re),
            export_eigen_sparse(this->engine, factory, basis_im)
        };
    }

    matlab::data::CellArray SymbolComboExporter::direct(const Polynomial &combo) const {

        matlab::data::ArrayFactory factory;
        auto output = factory.createCellArray({combo.size(), 1});
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
}
