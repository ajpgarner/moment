/**
 * read_polynomial.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/polynomial.h"

#include <string>
#include <vector>

namespace Moment::mex {

    struct raw_sc_data {
        uint64_t symbol_id;
        std::complex<double> factor;
        bool conjugated;
    };

    /**
     * Parse cell array into possible symbol combo data.
     */
    [[nodiscard]] std::vector<raw_sc_data>
    read_raw_polynomial_data(matlab::engine::MATLABEngine& engine,
                             const std::string& fieldName, const matlab::data::Array& input);

    /**
     * Parse cell array into possible symbol expression.
     */
    [[nodiscard]] raw_sc_data
    read_raw_monomial(matlab::engine::MATLABEngine& engine,
                      const std::string& fieldName, const matlab::data::Array& input);

    /**
     * Convert possible symbol combo data into an actual symbol combo, using factory.
     */
    [[nodiscard]] Polynomial raw_data_to_polynomial(const SymbolComboFactory& factory,
                                                    std::span<const raw_sc_data> data);

    /**
     * Read cell array directly into a symbol combo.
     */
    [[nodiscard]] inline Polynomial read_polynomial(matlab::engine::MATLABEngine& engine,
                                                    const std::string& fieldName,
                                                    const SymbolComboFactory& factory,
                                                    const matlab::data::Array& input) {
        auto raw_data = read_raw_polynomial_data(engine, fieldName, input);
        return raw_data_to_polynomial(factory, raw_data);
    }

}