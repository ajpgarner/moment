/**
 * format_factor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <complex>
#include <iosfwd>

namespace Moment {

    /**
     *
     * @param os The output stream to write to.
     * @param factor The factor to format.
     * @param is_scalar True if the factor is 'stand alone'
     * @param needs_plus True if we want a " + " or " - " preceeding factor.
     * @return True if a space is required after factor and before object.
     */
    bool format_factor(std::ostream& os, std::complex<double> factor, bool is_scalar, bool needs_plus);

}