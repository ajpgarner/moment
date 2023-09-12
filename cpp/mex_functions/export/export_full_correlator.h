/**
 * export_full_correlator.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "export_polynomial_tensor.h"

#include <span>

namespace Moment {
    class FullCorrelator;

    namespace mex {
        class FullCorrelatorExporter : public PolynomialTensorExporter {


        public:
            FullCorrelatorExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system)
                : PolynomialTensorExporter{engine, system} { }

        };

    }

}