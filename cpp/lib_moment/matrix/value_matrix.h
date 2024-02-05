/**
 * value_matrix.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
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

    private:
        bool antihermitian = false;

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


        std::unique_ptr<SymbolicMatrix>
        pre_multiply(const OperatorSequence& lhs, std::complex<double> weight, const PolynomialFactory& poly_factory,
                     SymbolTable& symbol_table, Multithreading::MultiThreadPolicy policy) const override;

        std::unique_ptr<SymbolicMatrix>
        post_multiply(const OperatorSequence& lhs, std::complex<double> weight, const PolynomialFactory& poly_factory,
                      SymbolTable& symbol_table, Multithreading::MultiThreadPolicy policy) const override;

        std::unique_ptr<SymbolicMatrix>
        pre_multiply(const RawPolynomial& lhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const override;

        std::unique_ptr<SymbolicMatrix>
        post_multiply(const RawPolynomial& rhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const override;

        [[nodiscard]] bool AntiHermitian() const noexcept {
            return this->antihermitian;
        }

    };
}