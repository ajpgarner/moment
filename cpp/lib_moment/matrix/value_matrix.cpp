/**
 * value_matrix.cpp
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "value_matrix.h"

#include "polynomial_matrix.h"

#include "dictionary/operator_sequence.h"
#include "dictionary/raw_polynomial.h"

#include "scenarios/context.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/eigen_utils.h"
#include "utilities/float_utils.h"

#include <algorithm>


namespace Moment {

    namespace {

        template<typename field_t>
        std::unique_ptr<SquareMatrix<Monomial>>
        to_monomial_matrix(const Eigen::Matrix<field_t, Eigen::Dynamic, Eigen::Dynamic>& data, double zero_tolerance) {
            if (data.rows() != data.cols()) {
                throw std::domain_error{"Matrix must be square."};
            }

            std::vector<Monomial> mono_data;
            mono_data.reserve(data.size());

            std::transform(data.data(), data.data() + data.size(), std::back_inserter(mono_data),
                           [zero_tolerance](auto val) -> Monomial {
                if (!approximately_zero(val, zero_tolerance)) {
                    return Monomial{1, val, false};
                } else {
                    return Monomial{0, 0.0, false};
                }
            });

            return std::make_unique<SquareMatrix<Monomial>>(data.rows(), std::move(mono_data));
        }


        template<typename field_t>
        std::unique_ptr<SquareMatrix<Monomial>>
        to_monomial_matrix(const Eigen::SparseMatrix<field_t>& data) {
            if (data.rows() != data.cols()) {
                throw std::domain_error{"Matrix must be square."};
            }

            using inner_iter_t = typename Eigen::SparseMatrix<field_t>::InnerIterator;
            const size_t elems = data.rows() * data.cols();
            std::vector<Monomial> mono_data(elems, Monomial{0, 0.0, false});

            // Cycle over non-zero elements and set:
            for (int outer_index = 0; outer_index < data.outerSize(); ++outer_index) {
                for (inner_iter_t inner_iter(data, outer_index); inner_iter; ++inner_iter) {
                    const size_t offset = inner_iter.col() * data.rows() + inner_iter.row();
                    mono_data[offset].id = 1;
                    mono_data[offset].factor = inner_iter.value();
                }
            }

            return std::make_unique<SquareMatrix<Monomial>>(data.rows(), std::move(mono_data));
        }
    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, const Eigen::MatrixXd& data,
                             std::optional<std::string> input_description)
         : MonomialMatrix{context, symbols, zero_tolerance,
                          to_monomial_matrix(data, zero_tolerance), is_hermitian(data, zero_tolerance)} {
        if (input_description.has_value()) {
            this->description = input_description.value();
        } else {
            this->description = "Real Value Matrix";
        }

        // Since matrix is real, it can only be anti-Hermitian if it is zero.
        if (is_zero(data)) {
            this->antihermitian = true;
        }

    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, const Eigen::MatrixXcd& data,
                             std::optional<std::string> input_description)
        : MonomialMatrix{context, symbols, zero_tolerance,
                         to_monomial_matrix(data, zero_tolerance), is_hermitian(data, zero_tolerance)} {
        if (input_description.has_value()) {
            this->description = input_description.value();
        } else {
            this->description = "Complex Value Matrix";
        }

        if (this->hermitian) {
            if (is_zero(data)) {
                this->antihermitian = true;
            }
        } else {
            this->antihermitian = is_antihermitian(data);
        }

    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, Eigen::SparseMatrix<double>& data,
                             std::optional<std::string> input_description)
            : MonomialMatrix{context, symbols, zero_tolerance,
                             to_monomial_matrix(data), is_hermitian(data, zero_tolerance)} {
        if (input_description.has_value()) {
            this->description = input_description.value();
        } else {
            this->description = "Real Value Matrix";
        }

        // Since matrix is real, it can only be anti-Hermitian if it is zero.
        if (is_zero(data)) {
            this->antihermitian = true;
        }

    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, Eigen::SparseMatrix<std::complex<double>>& data,
                             std::optional<std::string> input_description)
            : MonomialMatrix{context, symbols, zero_tolerance,
                             to_monomial_matrix(data), is_hermitian(data, zero_tolerance)} {

        if (input_description.has_value()) {
            this->description = input_description.value();
        } else {
            this->description = "Complex Value Matrix";
        }

        if (this->hermitian) {
            if (is_zero(data)) {
                this->antihermitian = true;
            }
        } else {
            this->antihermitian = is_antihermitian(data);
        }
    }

    namespace {
        Monomial monomialize(const OperatorSequence& lhs, std::complex<double> weight, SymbolTable& symbol_table) {
            // Rotate to real, if necessary
            if (lhs.get_sign() != SequenceSignType::Positive) [[unlikely]] {
                weight *= to_scalar(lhs.get_sign());
                OperatorSequence copy_lhs{lhs};
                copy_lhs.set_sign(SequenceSignType::Positive);
                return monomialize(copy_lhs, weight, symbol_table);
            }

            // Find existing symbol
            auto existing = symbol_table.where(lhs);
            if (existing.found()) {
                return Monomial{existing.symbol->Id(), weight, existing.is_conjugated};
            }

            // Register new symbol
            auto registered_id = symbol_table.merge_in(OperatorSequence{lhs});
            return Monomial{registered_id, weight, false};;

        }
    }

    std::unique_ptr<SymbolicMatrix> ValueMatrix::pre_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                                                              const PolynomialFactory& poly_factory,
                                                              SymbolTable& symbol_table,
                                                              Multithreading::MultiThreadPolicy policy) const {



        if ((approximately_zero(weight, poly_factory.zero_tolerance)) || (lhs.zero())) {
            return MonomialMatrix::zero_matrix(this->context, symbol_table, this->dimension);
        } else {

            const Monomial mono_lhs = monomialize(lhs, weight, symbol_table);

            std::vector<Monomial> matrix_data;
            matrix_data.reserve(this->dimension * this->dimension);
            for (const auto& mono_rhs : this->SymbolMatrix()) {
                std::complex<double> new_factor = mono_lhs.factor * mono_rhs.factor;
                if (!approximately_zero(new_factor, poly_factory.zero_tolerance)) {
                    matrix_data.emplace_back(mono_lhs.id, new_factor, mono_lhs.conjugated);
                } else {
                    matrix_data.emplace_back(0, 0.0, false);
                }
            }
            auto mat_data_ptr = std::make_unique<SquareMatrix<Monomial>>(this->dimension, std::move(matrix_data));

            // Deduce if Hermitian
            const bool new_matrix_hermitian = [&]() {
                if (this->hermitian) {
                    auto lhs_ht = lhs.hermitian_type();
                    return (lhs_ht == HermitianType::Hermitian);
                } else if (this->antihermitian) {
                    auto lhs_ht = lhs.hermitian_type();
                    return (lhs_ht == HermitianType::AntiHermitian);
                } else {
                    return false;
                }
            }();

            // Create new matrix
            return std::make_unique<MonomialMatrix>(this->context, symbol_table, poly_factory.zero_tolerance,
                                                      std::move(mat_data_ptr), new_matrix_hermitian);

        }
    }

    std::unique_ptr<SymbolicMatrix> ValueMatrix::post_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                                                               const PolynomialFactory& poly_factory,
                                                               SymbolTable& symbol_table,
                                                               Multithreading::MultiThreadPolicy policy) const {
        // Element-wise multiplication with scalar matrix commutes:
        return ValueMatrix::pre_multiply(lhs, weight, poly_factory, symbol_table, policy);
    }

    std::unique_ptr<SymbolicMatrix>
    ValueMatrix::pre_multiply(const RawPolynomial& lhs, const PolynomialFactory& poly_factory,
                              SymbolTable& symbol_table, Multithreading::MultiThreadPolicy policy) const {
        if (lhs.empty()) {
            return MonomialMatrix::zero_matrix(this->context, symbol_table, this->dimension);
        } else if (lhs.size() == 1) {
            // Delegate to OperatorSequence function
            const auto& elem = *lhs.begin();
            return ValueMatrix::pre_multiply(elem.sequence, elem.weight, poly_factory, symbol_table, policy);
        } else {
            // Make sure LHS is not scalar in disguise
            if (lhs.is_scalar()) {
                std::complex<double> cumulative_weight = 0;
                for (const auto& elem : lhs) {
                    cumulative_weight += elem.weight;
                }
                return ValueMatrix::pre_multiply(OperatorSequence::Identity(this->context), cumulative_weight,
                                                 poly_factory, symbol_table, policy);
            }

            // Otherwise, instantiate LHS polynomial
            const auto symbolized_poly = lhs.to_polynomial_register_symbols(poly_factory, symbol_table);

            // Copy LHS polynomial up to scaling factors from this matrix
            std::vector<Polynomial> matrix_data;
            matrix_data.reserve(this->dimension * this->dimension);
            for (const auto& monomial : this->SymbolMatrix()) {
                matrix_data.emplace_back(poly_factory.scale(symbolized_poly, monomial.factor));
            }
            auto mat_data_ptr = std::make_unique<SquareMatrix<Polynomial>>(this->dimension, std::move(matrix_data));

            return std::make_unique<PolynomialMatrix>(this->context, symbol_table, poly_factory.zero_tolerance,
                                                      std::move(mat_data_ptr));


        }
    }

    std::unique_ptr<SymbolicMatrix>
    ValueMatrix::post_multiply(const RawPolynomial& rhs, const PolynomialFactory& poly_factory,
                               SymbolTable& symbol_table, Multithreading::MultiThreadPolicy policy) const {
        // Element-wise multiplication with scalar matrix commutes:
        return ValueMatrix::pre_multiply(rhs, poly_factory, symbol_table, policy);
    }


}