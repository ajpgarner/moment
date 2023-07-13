/**
 * polynomial_localizing_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_localizing_matrix_index.h"
#include "symbolic/symbol_table.h"

namespace Moment {

    std::pair<LocalizingMatrixIndex, std::complex<double>>
    PolynomialLMIndex::MonomialLMIterator::operator*() const noexcept {
        assert(symbolPtr);
        const auto& monomial = *this->iter;
        assert(monomial.id >= 0 && monomial.id < symbolPtr->size());
        const auto& symbolInfo = (*this->symbolPtr)[monomial.id];
        const auto& opSeqRef = monomial.conjugated ? symbolInfo.sequence_conj() : symbolInfo.sequence();
        return {LocalizingMatrixIndex(this->level, opSeqRef), monomial.factor};
    }

}