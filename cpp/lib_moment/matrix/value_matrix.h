/**
 * value_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "monomial_matrix.h"

#include <optional>
#include <string>


namespace Moment {

    /**
     * Symbolic matrix, where each entry represents a scalar value multiplied by <I>.
     */
    class ValueMatrix : public MonomialMatrix {
    public:
        /** Construct value matrix from dense real Eigen matrix. */
        ValueMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                    const Eigen::MatrixXd& data, std::optional<std::string> description = std::nullopt);

        /** Construct value matrix from dense complex Eigen matrix. */
        ValueMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                    const Eigen::MatrixXcd& data, std::optional<std::string> description = std::nullopt);

        /** Construct value matrix from sparse real Eigen matrix. */
        ValueMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                    Eigen::SparseMatrix<double>& data, std::optional<std::string> description = std::nullopt);

        /** Construct value matrix from sparse complex Eigen matrix. */
        ValueMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                    Eigen::SparseMatrix<std::complex<double>>& data,
                    std::optional<std::string> description = std::nullopt);


    };
}