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
     * Convert possible symbol combo data into an actual polynomial, using factory.
     * @throws matlab::Exception (errors::bad_param) if symbols are out of bounds.
     */
    [[nodiscard]] Polynomial raw_data_to_polynomial(matlab::engine::MATLABEngine& engine,
                                                    const PolynomialFactory& factory,
                                                    std::span<const raw_sc_data> data);

    /**
     * Convert possible symbol combo data into an actual polynomial, assuming pre-sorted.
     * @throws matlab::Exception (errors::bad_param) if symbols are out of bounds.
     */
    [[nodiscard]] Polynomial raw_data_to_polynomial_assume_sorted(matlab::engine::MATLABEngine& engine,
                                                                  const PolynomialFactory& factory,
                                                                  std::span<const raw_sc_data> data);

    /**
     * Do bounds check on symbol combo data.
     * @throws matlab::Exception (errors::bad_param) if symbols are out of bounds.
     */
    void check_raw_polynomial_data(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols,
                                   std::span<const raw_sc_data> data);

    /**
     * Read cell array directly into a symbol combo.
     */
    [[nodiscard]] inline Polynomial read_polynomial(matlab::engine::MATLABEngine& engine,
                                                    const std::string& fieldName,
                                                    const PolynomialFactory& factory,
                                                    const matlab::data::Array& input) {
        auto raw_data = read_raw_polynomial_data(engine, fieldName, input);
        return raw_data_to_polynomial(engine, factory, raw_data);
    }

}