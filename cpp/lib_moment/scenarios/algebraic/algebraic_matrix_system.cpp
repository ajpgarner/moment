/**
 * algebraic_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_matrix_system.h"
#include "algebraic_context.h"

#include "symbolic/monomial_comparator_by_hash.h"

namespace Moment::Algebraic {

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class AlgebraicContext> contextIn,
                                                 const double zero_tolerance)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {

        this->replace_polynomial_factory(
            std::make_unique<ByHashPolynomialFactory>(this->Symbols(), zero_tolerance, this->Symbols())
        );
    }

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class Context> contextIn,
                                                 const double zero_tolerance)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {
        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), zero_tolerance, this->Symbols())
        );
    }
}
