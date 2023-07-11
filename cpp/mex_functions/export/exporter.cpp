/**
 * exporter.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "exporter.h"

#include "utilities/reporting.h"
#include "error_codes.h"

namespace Moment::mex {

    void Exporter::report_too_small_output() const {
        throw_error(this->engine, errors::internal_error,
                    "End of output unexpectedly encountered before read was finished.");
    }

    void Exporter::report_too_small_input() const {
        throw_error(this->engine, errors::internal_error,
                    "End of input unexpectedly encountered before write was finished.");
    }


}