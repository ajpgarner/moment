/**
 * lu_map_core_processor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "lu_map_core_processor.h"

#include <Eigen/LU>
#include <Eigen/SparseLU>

#include <algorithm>

namespace Moment::Derived {
    LUMapCoreProcessor::~LUMapCoreProcessor() noexcept = default;

    std::unique_ptr<SolvedMapCore> LUMapCoreProcessor::operator()(const DenseMapCore &core) const {
        auto solutionPtr = std::make_unique<SolvedMapCore>();
        auto& solution = *solutionPtr;

        const Eigen::Index input_cols = core.core.cols();
        const Eigen::Index input_rows = core.core.rows();

        if ((core.core.cols() == 0) || (core.core.rows() == 0)) {
            solution.trivial_solution = true;
            solution.dense_map.resize(0, 0);
            solution.dense_inv_map.resize(0, 0);
            return solutionPtr;
        }

        // Decompose matrix
        Eigen::FullPivLU<Eigen::MatrixXd> lu{core.core};
        const Eigen::Index rank = lu.rank();
        solution.output_symbols = static_cast<size_t>(rank);
        solution.trivial_solution = false;
        solution.dense_solution = true;

        // Otherwise, we have non-trivial rank reduction; extract L and U matrices as map and inverse map.
        solution.dense_map = Eigen::MatrixXd::Identity(input_rows, rank);
        solution.dense_map.triangularView<Eigen::StrictlyLower>() = lu.matrixLU().topLeftCorner(input_rows, rank);
        solution.dense_map = lu.permutationP().inverse() * solution.dense_map;

        solution.dense_inv_map = lu.matrixLU().topLeftCorner(rank, input_cols).triangularView<Eigen::Upper>();
        solution.dense_inv_map = solution.dense_inv_map * lu.permutationQ().inverse();

        return solutionPtr;
    }

    std::unique_ptr<SolvedMapCore> LUMapCoreProcessor::operator()(const SparseMapCore &core) const {
        auto solutionPtr = std::make_unique<SolvedMapCore>();
        auto& solution = *solutionPtr;

        const Eigen::Index input_cols = core.core.cols();
        const Eigen::Index input_rows = core.core.rows();

        if ((core.core.cols() == 0) || (core.core.rows() == 0)) {
            solution.trivial_solution = true;
            solution.sparse_solution = true;
            solution.sparse_map.resize(0, 0);
            solution.sparse_inv_map.resize(0, 0);
            return solutionPtr;
        }

       // Rational: Still looking for suitable rank-revealing LU decomposition for sparse matrices.
       //           Eigen's built-in solvers are only for full-rank (i.e. useless here!) decompositions.
       throw std::logic_error{"LUMapCoreProcessor::operator()(const SparseMapCore &core) is currently not supported."};

    }
}