/**
 * export_symbol_combo.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_symbol_combo.h"

#include "symbolic/symbol_combo.h"
#include "symbolic/symbol_combo_to_basis.h"
#include "symbolic/symbol_table.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

#include "eigen/export_eigen_sparse.h"

#include "error_codes.h"

namespace Moment::mex {

    std::pair<matlab::data::Array, matlab::data::Array>
        SymbolComboExporter::operator()(const SymbolCombo& combo) const {

        auto [basis_re, basis_im] = SymbolComboToBasisVec{symbols}(combo);

        matlab::data::ArrayFactory factory;

        return {
            export_eigen_sparse(this->engine, factory, basis_re),
            export_eigen_sparse(this->engine, factory, basis_im)
        };
    }
}
