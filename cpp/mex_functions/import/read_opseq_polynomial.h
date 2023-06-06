/**
 * read_opseq_polynomial.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <complex>
#include <optional>
#include <string>

#include "symbolic/polynomial.h"

#include "MatlabDataArray.hpp"


namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class Context;
    class SymbolTable;
}

namespace Moment::mex {

    class StagingMonomial;

    class StagingPolynomial {
    private:
        matlab::engine::MATLABEngine& matlabEngine;

        std::string name;

    public:
        explicit StagingPolynomial(matlab::engine::MATLABEngine& engine,
                                   const matlab::data::Array& input,
                                   std::string inputName);

        ~StagingPolynomial() noexcept;

        void supply_context(const Context& context);

        void find_symbols(const SymbolTable& symbols);

        void find_or_register_symbols(SymbolTable& symbols);

        [[nodiscard]] Polynomial to_polynomial(const PolynomialFactory& factory) const;

    private:
        std::unique_ptr<StagingMonomial[]> data;

        size_t data_length = 0;

        bool symbols_resolved = false;
    };



}
