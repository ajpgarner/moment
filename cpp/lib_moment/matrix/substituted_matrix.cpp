/**
 * substituted_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substituted_matrix.h"

#include "symbolic/polynomial_factory.h"

#include <sstream>

namespace Moment {
    namespace {
       inline SymbolTable& assert_symbols(SymbolTable& symbols, const Matrix& the_source) {
            assert(&symbols == &the_source.symbols);
            return symbols;
        }
    }

    std::string SubstitutedMatrix::make_name() {
        std::stringstream ss;
        ss << "Substituted Matrix [Source: " << this->source_matrix.Description() << "; Rules: " << rules.name() << "]";
        return ss.str();
    }

    MonomialSubstitutedMatrix::MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                         const MonomialMatrix& the_source)
         : MonomialMatrix{the_source.context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                          MonomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix()),
                          the_source.Hermitian() && msrb.is_hermitian()},
           SubstitutedMatrix{the_source, msrb} {

        this->description = this->make_name();
    }

    std::unique_ptr<SquareMatrix<Monomial>>
    MonomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                       const SquareMatrix<Monomial>& matrix) {
        SquareMatrix<Monomial>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& expr : matrix) {
            data.emplace_back(msrb.reduce_monomial(expr));
        }
        return std::make_unique<SquareMatrix<Monomial>>(matrix.dimension, std::move(data));
    }


    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                             const MonomialMatrix& the_source)
         : PolynomialMatrix{context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix())},
           SubstitutedMatrix{the_source, msrb}  {
        this->description = this->make_name();
    }

    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                                             const PolynomialMatrix& the_source)
         : PolynomialMatrix{context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix())},
           SubstitutedMatrix{the_source, msrb} {
        this->description = this->make_name();
    }

    std::unique_ptr<SquareMatrix<Polynomial>>
    PolynomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                         const SquareMatrix<Polynomial>& matrix) {
        SquareMatrix<Polynomial>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& combo : matrix) {
            data.emplace_back(msrb.reduce(combo));
        }
        return std::make_unique<SquareMatrix<Polynomial>>(matrix.dimension, std::move(data));
    }

    std::unique_ptr<SquareMatrix<Polynomial>>
    PolynomialSubstitutedMatrix::reduce( const MomentSubstitutionRulebook& msrb,
                                         const SquareMatrix<Monomial>& matrix) {
        SquareMatrix<Polynomial>::StorageType data;
        data.reserve(matrix.dimension * matrix.dimension);
        for (const auto& expr : matrix) {
            data.emplace_back(msrb.reduce(expr));
        }
        return std::make_unique<SquareMatrix<Polynomial>>(matrix.dimension, std::move(data));
    }

}