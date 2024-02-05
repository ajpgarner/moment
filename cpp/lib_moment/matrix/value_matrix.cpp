/**
 * value_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "value_matrix.h"

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
                             double zero_tolerance, const Eigen::MatrixXd& data)
         : MonomialMatrix{context, symbols, zero_tolerance,
                          to_monomial_matrix(data, zero_tolerance), is_hermitian(data, zero_tolerance)} {
        this->description = "Real Value Matrix";

    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, const Eigen::MatrixXcd& data)
        : MonomialMatrix{context, symbols, zero_tolerance,
                         to_monomial_matrix(data, zero_tolerance), is_hermitian(data, zero_tolerance)} {
        this->description = "Complex Value Matrix";
    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, Eigen::SparseMatrix<double>& data)
            : MonomialMatrix{context, symbols, zero_tolerance,
                             to_monomial_matrix(data), is_hermitian(data, zero_tolerance)} {
        this->description = "Real Value Matrix";
    }

    /** Construct precomputed monomial matrix without operator matrix. */
    ValueMatrix::ValueMatrix(const Context& context, SymbolTable& symbols,
                             double zero_tolerance, Eigen::SparseMatrix<std::complex<double>>& data)
            : MonomialMatrix{context, symbols, zero_tolerance,
                             to_monomial_matrix(data), is_hermitian(data, zero_tolerance)} {

        this->description = "Complex Value Matrix";
    }



}