/**
 * map_core.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "map_core.h"

#include "derived_errors.h"

#include "symbolic/symbol_table.h"

#include "utilities/float_utils.h"

#include <cmath>

#include <limits>
#include <sstream>

namespace Moment::Derived {

    MapCore::MapCore(const size_t initial_src_size, const size_t initial_target_size, DynamicBitset<size_t> skipped)
            : initial_size{initial_src_size},
              nontrivial_rows{initial_target_size, false}, nontrivial_cols{initial_src_size, true},
              skipped_cols{std::move(skipped)} {

        if (initial_src_size < 1) {
            throw errors::bad_map{"Map must have action on identity (col #0)."};
        }
        if (initial_target_size < 1) {
            throw errors::bad_map{"Map must specify identity (row #0)."};
        }

    }

    std::pair<Eigen::Index, Eigen::Index>
    MapCore::identify_nontrivial(const Eigen::MatrixXd& input_dense, double eps_mult) {

        this->nontrivial_cols[0] = false;
        this->nontrivial_rows[0] = true;

        // Check first column maps ID->ID
        if (!approximately_equal(input_dense.coeff(0,0), 1.0)) {
            throw errors::bad_map{"First column of transformation must map identity to the identity."};
        }
        auto col_has_any_non_constant = [&input_dense, eps_mult](Eigen::Index col) -> bool {
            auto row_iter = Eigen::MatrixXd::InnerIterator{input_dense, col};
            ++row_iter;
            while (row_iter) {
                if (!approximately_zero(row_iter.value(), eps_mult)) {
                    return true;
                }
                ++row_iter;
            }
            return false;
        };
        if (col_has_any_non_constant(0)) {
            throw errors::bad_map{"First column of transformation must map identity to the identity."};
        }

        for (int col_index = 1; col_index < input_dense.cols(); ++col_index) {
            // Skip columns (and mark as trivial)
            if (this->skipped_cols.test(col_index)) {
                this->nontrivial_cols[col_index] = false;
                continue;
            }


            // Identify rows with no values, or only a constant value:
            const bool hasConstant = !approximately_zero(input_dense(0, col_index), eps_mult);
            const bool hasAnythingElse = col_has_any_non_constant(col_index);
            if (!hasAnythingElse) {
                if (!hasConstant) {
                    this->constants.emplace(std::make_pair(col_index, 0));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                } else {
                    const double offset_term = input_dense.coeff(0, col_index);
                    this->constants.emplace(std::make_pair(col_index, offset_term));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                }
            }

            // Otherwise, column is nontrivial - identify rows that are nontrivial
            for (auto row_iter = Eigen::MatrixXd::InnerIterator{input_dense, col_index};
                 row_iter; ++row_iter) {
                if (!approximately_zero(row_iter.value(), eps_mult)) {
                    this->nontrivial_rows[row_iter.row()] = true;
                }
            }
        }

        // Constant offset handled separately...
        this->nontrivial_rows[0] = false;

        return {static_cast<Eigen::Index>(this->nontrivial_cols.count()),
                static_cast<Eigen::Index>(this->nontrivial_rows.count())};
    }

    std::pair<Eigen::Index, Eigen::Index>
    MapCore::identify_nontrivial(const Eigen::SparseMatrix<double>& input_sparse) {

        this->nontrivial_cols[0] = false;
        this->nontrivial_rows[0] = true;

        // Check first column maps ID->ID
        if ((input_sparse.col(0).nonZeros() != 1) || !approximately_equal(input_sparse.coeff(0,0), 1.0)) {
            throw errors::bad_map{"First column of transformation must map identity to the identity."};
        }

        for (int col_index = 1; col_index < input_sparse.cols(); ++col_index) {

            // Skip columns (and mark as trivial)
            if (this->skipped_cols.test(col_index)) {
                this->nontrivial_cols[col_index] = false;
                continue;
            }

            // Identify rows with no values, or only a constant value:
            Eigen::Index nnz = input_sparse.col(col_index).nonZeros();
            if (0 == nnz) {
                this->constants.emplace(std::make_pair(static_cast<size_t>(col_index), 0));
                this->nontrivial_cols[col_index] = false;
                continue;
            } else if (1 == nnz) {
                const double offset_term = input_sparse.coeff(0, col_index);
                if (offset_term > 0) {
                    this->constants.emplace(std::make_pair(static_cast<size_t>(col_index), offset_term));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                }
            }

            // Otherwise, column is nontrivial - identify rows that are nontrivial
            for (auto row_iter = Eigen::SparseMatrix<double>::InnerIterator{input_sparse, col_index};
                 row_iter; ++row_iter) {
                this->nontrivial_rows[row_iter.row()] = true;
            }
        }

        // Constant offset handled separately...
        this->nontrivial_rows[0] = false;

        return {static_cast<Eigen::Index>(this->nontrivial_cols.count()),
                static_cast<Eigen::Index>(this->nontrivial_rows.count())};
    }


    // Make map from 'old' index to new
    std::vector<Eigen::Index> MapCore::remap_vector(DynamicBitset<size_t> nontrivial)  {
        std::vector<Eigen::Index> row_map(nontrivial.bit_size, -1);
        Eigen::Index new_row_idx = 0;
        for (const auto old_row_idx:nontrivial) {
            row_map[old_row_idx] = new_row_idx;
            ++new_row_idx;
        }
        return row_map;
    }


    void MapCore::do_check_solution(const size_t outer_rows, const size_t outer_cols,
                                    const SolvedMapCore& solution) const {
        if (solution.dense_solution) {
            if (outer_rows != solution.dense_map.rows()) {
                std::stringstream errSS;
                errSS << "MapCore has " << outer_rows << ((outer_rows != 1) ? " rows" : " row")
                      << ", which does not match with SolvedMapCore map's "
                      << solution.dense_map.rows() << ((solution.dense_map.rows() != 1) ? " rows" : " row") << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.dense_map.cols() != solution.output_symbols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore map has " << solution.dense_map.cols()
                      << ((solution.dense_map.cols() != 1) ? " columns" : " column")
                      << ", which does not match declared map rank " << solution.output_symbols << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.dense_inv_map.rows() != solution.output_symbols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore inverse map has " << solution.dense_inv_map.rows()
                      << ((solution.dense_inv_map.rows() != 1) ? " rows" : " row")
                      << ", which does not match declared map rank " << solution.output_symbols << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.dense_inv_map.cols() != outer_cols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore inverse map has "
                      << solution.dense_inv_map.cols()
                      << ((solution.dense_inv_map.cols() != 1) ? " columns" : " column")
                      << ", which does not match with MapCore's "
                      << outer_cols << ((outer_cols != 1) ? " columns" : " column") << ".";
                throw errors::invalid_solution{errSS.str()};
            }
        }
        if (solution.sparse_solution) {
            if (outer_rows != solution.sparse_map.rows()) {
                std::stringstream errSS;
                errSS << "MapCore has " << outer_rows << ((outer_rows != 1) ? " rows" : " row")
                      << ", which does not match with SolvedMapCore sparse map's "
                      << solution.sparse_map.rows() << ((solution.sparse_map.rows() != 1) ? " rows" : " row") << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.sparse_map.cols() != solution.output_symbols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore sparse map has " << solution.sparse_map.cols()
                      << ((solution.sparse_map.cols() != 1) ? " columns" : " column")
                      << ", which does not match declared map rank " << solution.output_symbols << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.sparse_inv_map.rows() != solution.output_symbols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore sparse inverse map has " << solution.sparse_inv_map.rows()
                      << ((solution.sparse_inv_map.rows() != 1) ? " rows" : " row")
                      << ", which does not match declared map rank " << solution.output_symbols << ".";
                throw errors::invalid_solution{errSS.str()};
            }

            if (solution.sparse_inv_map.cols() != outer_cols) {
                std::stringstream errSS;
                errSS << "SolvedMapCore sparse inverse map has "
                      << solution.sparse_inv_map.cols()
                      << ((solution.sparse_inv_map.cols() != 1) ? " columns" : " column")
                      << ", which does not match with MapCore's "
                      << outer_cols << ((outer_cols != 1) ? " columns" : " column") << ".";
                throw errors::invalid_solution{errSS.str()};
            }
        }
    }


    DenseMapCore::DenseMapCore(DynamicBitset<size_t> skipped, const Eigen::MatrixXd &raw_remap, const double zero_epsilon)
        : MapCore{static_cast<size_t>(raw_remap.cols()), static_cast<size_t>(raw_remap.rows()), std::move(skipped)} {

        const auto [remapped_cols, remapped_rows] = this->identify_nontrivial(raw_remap, zero_epsilon);

        // Copy dense matrix into dense matrix (extracting constant row separately) and pruning close to zero with zero.
        this->core_offset.resize(remapped_cols);
        this->core.resize(remapped_cols, remapped_rows);

        Eigen::Index new_col_idx = 0;
        for (const auto old_col_idx : this->nontrivial_cols) {
            double c_read_val = raw_remap.coeff(0, static_cast<Eigen::Index>(old_col_idx));
            this->core_offset(new_col_idx) = approximately_zero(c_read_val, zero_epsilon) ? 0.0 : c_read_val;


            Eigen::Index new_row_idx = 0;
            for (const auto old_row_idx : this->nontrivial_rows) {
                double read_val = raw_remap.coeff(static_cast<Eigen::Index>(old_row_idx),
                                                    static_cast<Eigen::Index>(old_col_idx));

                // Do transpose
                this->core(new_col_idx, new_row_idx) = approximately_zero(read_val, zero_epsilon) ? 0.0 : read_val;
                ++new_row_idx;
            }
            ++new_col_idx;
        }
    }


    DenseMapCore::DenseMapCore(DynamicBitset<size_t> skipped, const Eigen::SparseMatrix<double>& raw_remap)
            : MapCore{static_cast<size_t>(raw_remap.cols()), static_cast<size_t>(raw_remap.rows()), std::move(skipped)} {

        const auto [remapped_cols, remapped_rows] = this->identify_nontrivial(raw_remap);

        // Make random-access map from 'old' index to new
        const std::vector<Eigen::Index> row_remap = MapCore::remap_vector(this->nontrivial_rows);

        // Prepare core and offset
        this->core_offset.resize(remapped_cols);
        this->core_offset.setZero();
        this->core.resize(remapped_cols, remapped_rows);
        this->core.setZero();

        Eigen::Index new_col_idx = 0;
        for (const auto old_col_idx : this->nontrivial_cols) {
            // First, do we have constant?
            Eigen::SparseMatrix<double>::InnerIterator iter_over_rows{raw_remap,
                                                                      static_cast<Eigen::Index>(old_col_idx)};

            assert(iter_over_rows); // Should not be empty!

            // Handle constant offset, if any
            const bool has_offset = (iter_over_rows.index() == 0);
            if (has_offset) {
                this->core_offset[new_col_idx] = iter_over_rows.value();
                ++iter_over_rows;
            }

            // Rest of row, if any
            while(iter_over_rows) {
                // Do transpose
                this->core(new_col_idx, row_remap[iter_over_rows.index()]) = iter_over_rows.value();
                ++iter_over_rows;
            }
            ++new_col_idx;
        }
    }


    void DenseMapCore::check_solution(const SolvedMapCore& solution) const {
        this->do_check_solution(this->core.rows(), this->core.cols(), solution);
    }


    /**
     * Extracts core of map.
     * @param skipped Columns to skip from the transformation matrix.
     * @param matrix The input transformation matrix.
     * @param zero_tolerance The value below which an entry is treated as zero.
     */
    SparseMapCore::SparseMapCore(DynamicBitset<size_t> skipped, const Eigen::MatrixXd& raw_remap, const double zero_tolerance)
        : MapCore{static_cast<size_t>(raw_remap.cols()), static_cast<size_t>(raw_remap.rows()), std::move(skipped)} {

        const auto [remapped_cols, remapped_rows] = this->identify_nontrivial(raw_remap, zero_tolerance);

        // Prepare core and offset
        this->core_offset.resize(remapped_cols);
        this->core_offset.setZero();
        this->core.resize(remapped_cols, remapped_rows);

        // Make random-access map from 'old' index to new
        const std::vector<Eigen::Index> row_remap = MapCore::remap_vector(this->nontrivial_rows);

        std::vector<Eigen::Triplet<double>> triplets;
        Eigen::Index new_col_idx = 0;
        for (const auto old_col_idx : this->nontrivial_cols) {

            // Handle constant offset, if any
            const bool has_offset = !approximately_zero(raw_remap(0, static_cast<Eigen::Index>(old_col_idx)), zero_tolerance);
            if (has_offset) {
                this->core_offset[new_col_idx] = raw_remap(0, static_cast<Eigen::Index>(old_col_idx));
            }

            // Rest of row, if any
            for (const auto old_row_idx : this->nontrivial_rows) {
                const double value = raw_remap(static_cast<Eigen::Index>(old_row_idx),
                                               static_cast<Eigen::Index>(old_col_idx));
                triplets.emplace_back(new_col_idx, row_remap[old_row_idx], value);
            }
            ++new_col_idx;
        }

        this->core.setFromTriplets(triplets.begin(), triplets.end());
    }

    /**
    * Extracts core of map.
    * @param skipped Columns to skip from the transformation matrix.
    * @param matrix The input transformation sparse matrix.
    */
    SparseMapCore::SparseMapCore(DynamicBitset<size_t> skipped, const Eigen::SparseMatrix<double>& raw_remap)
        : MapCore{static_cast<size_t>(raw_remap.cols()), static_cast<size_t>(raw_remap.rows()), std::move(skipped)} {

        const auto [remapped_cols, remapped_rows] = this->identify_nontrivial(raw_remap);

        // Make random-access map from 'old' index to new
        const std::vector<Eigen::Index> row_remap = MapCore::remap_vector(this->nontrivial_rows);

        // Prepare core and offset
        this->core_offset.resize(remapped_cols);
        this->core_offset.setZero();
        this->core.resize(remapped_cols, remapped_rows);

        std::vector<Eigen::Triplet<double>> triplets;
        Eigen::Index new_col_idx = 0;
        for (const auto old_col_idx : this->nontrivial_cols) {
            // First, do we have constant?
            Eigen::SparseMatrix<double>::InnerIterator iter_over_rows{raw_remap,
                                                                      static_cast<Eigen::Index>(old_col_idx)};

            assert(iter_over_rows); // Should not be empty!

            // Handle constant offset, if any
            const bool has_offset = (iter_over_rows.index() == 0);
            if (has_offset) {
                this->core_offset[new_col_idx] = iter_over_rows.value();
                ++iter_over_rows;
            }

            // Rest of row, if any
            while(iter_over_rows) {
                triplets.emplace_back(new_col_idx, row_remap[iter_over_rows.index()], iter_over_rows.value());
                ++iter_over_rows;
            }
            ++new_col_idx;
        }

        this->core.setFromTriplets(triplets.begin(), triplets.end());
    }

    void SparseMapCore::check_solution(const SolvedMapCore& solution) const {
        this->do_check_solution(this->core.rows(), this->core.cols(), solution);
    }
}