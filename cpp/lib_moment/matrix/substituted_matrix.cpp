/**
 * substituted_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substituted_matrix.h"

#include <sstream>

namespace Moment {
    namespace {
       inline SymbolTable& assert_symbols(SymbolTable& symbols, const Matrix& the_source) {
            assert(&symbols == &the_source.Symbols);
            return symbols;
        }
    }

    MonomialSubstitutedMatrix::MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                         const MonomialMatrix& the_source)
         : MonomialMatrix{the_source.context, assert_symbols(symbols, the_source),
                          MonomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix()),
                          the_source.is_hermitian() && msrb.is_hermitian()},
           SubstitutedMatrix{the_source, msrb} {
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    MonomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                       const SquareMatrix<SymbolExpression>& matrix) {
        SquareMatrix<SymbolExpression>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& expr : matrix) {
            data.emplace_back(msrb.reduce_monomial(expr));
        }
        return std::make_unique<SquareMatrix<SymbolExpression>>(matrix.dimension, std::move(data));
    }


    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                             const MonomialMatrix& the_source)
         : PolynomialMatrix{context, assert_symbols(symbols, the_source),
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix())},
           SubstitutedMatrix{the_source, msrb}  {
    }

    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                             const PolynomialMatrix& the_source)
         : PolynomialMatrix{context, assert_symbols(symbols, the_source),
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix())},
           SubstitutedMatrix{the_source, msrb} {
    }

    std::unique_ptr<SquareMatrix<SymbolCombo>>
    PolynomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                         const SquareMatrix<SymbolCombo>& matrix) {
        SquareMatrix<SymbolCombo>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& combo : matrix) {
            data.emplace_back(msrb.reduce(combo));
        }
        return std::make_unique<SquareMatrix<SymbolCombo>>(matrix.dimension, std::move(data));
    }

    std::unique_ptr<SquareMatrix<SymbolCombo>>
    PolynomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                         const SquareMatrix<SymbolExpression>& matrix) {
        SquareMatrix<SymbolCombo>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& expr : matrix) {
            data.emplace_back(msrb.reduce(expr));
        }
        return std::make_unique<SquareMatrix<SymbolCombo>>(matrix.dimension, std::move(data));
    }

}