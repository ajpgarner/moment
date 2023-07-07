/**
 * virtual_polynomial_view.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "virtual_polynomial_view.h"
#include "collins_gisin.h"

namespace Moment {
    std::pair<OperatorSequence, std::complex<double>>
    VirtualPolynomialView::VPVIter::operator*() const {
        assert(this->poly_iter->id >= 1);
        size_t offset = this->poly_iter->id-1;
        const auto cgEntry = this->view.collins_gisin.at(offset);
        return {cgEntry->sequence, this->poly_iter->factor};
    }
}