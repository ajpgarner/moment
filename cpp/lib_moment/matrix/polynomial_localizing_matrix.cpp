/**
 * polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_localizing_matrix.h"

#include "dictionary/raw_polynomial.h"

#include "matrix_system/matrix_system.h"

#include "dictionary/dictionary.h"
#include "scenarios/context.h"
#include "symbolic/polynomial_factory.h"

#include <cassert>
#include <stdexcept>

namespace Moment {

    namespace {

        std::string make_description(const Context& context, const SymbolTable& symbols,
                                     const PolynomialLMIndex& index) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << "Localizing Matrix, Level " << index.Level
                << ", Phrase " << index.Polynomial;

            return ss.str();
        }
    }


    PolynomialLocalizingMatrix::PolynomialLocalizingMatrix(const Context& context, SymbolTable& symbols,
                                                           const PolynomialFactory& factory,
                                                           PolynomialLMIndex index_in,
                                                           CompositeMatrix::ConstituentInfo&& constituents_in)
           : CompositeMatrix{context, symbols, factory, std::move(constituents_in)}, index{std::move(index_in)} {
        this->description = make_description(context, symbols, this->index);
    }

    std::unique_ptr<PolynomialLocalizingMatrix>
    PolynomialLocalizingMatrix::create_from_raw(const MaintainsMutex::WriteLock& write_lock, MatrixSystem& system,
                                                size_t level, const RawPolynomial& raw_polynomials,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(write_lock));

        // TODO: Implement
        throw std::logic_error{"PolynomialLocalizingMatrix::create_from_raw not implemented yet."};
    }

}