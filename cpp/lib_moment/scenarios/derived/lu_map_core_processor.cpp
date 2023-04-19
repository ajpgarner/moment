/**
 * lu_map_core_processor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "lu_map_core_processor.h"

#include <Eigen/LU>

#include <algorithm>
#include <iostream>

namespace Moment::Derived {

    std::unique_ptr<SolvedMapCore> LUMapCoreProcessor::operator()(const MapCore &core) {
        auto solutionPtr = std::make_unique<SolvedMapCore>();
        auto& solution = *solutionPtr;

        const Eigen::Index input_cols = core.core.cols();
        const Eigen::Index input_rows = core.core.rows();

        if ((core.core.cols() == 0) || (core.core.rows() == 0)) {
            solution.trivial_solution = true;
            solution.map.resize(0, 0);
            solution.inv_map.resize(0, 0);
            return solutionPtr;
        }

        // Decompose matrix
        Eigen::FullPivLU<Eigen::MatrixXd> lu{core.core.transpose()};
        const Eigen::Index rank = lu.rank();
        solution.output_symbols = static_cast<size_t>(rank);
        solution.trivial_solution = false;

        // Otherwise, we have non-trivial rank reduction; extract L and U matrices as map and inverse map.
        solution.map = Eigen::MatrixXd::Identity(input_rows, rank);
        solution.map.triangularView<Eigen::StrictlyLower>() = lu.matrixLU().topLeftCorner(input_rows, rank);
        solution.map = lu.permutationP().inverse() * solution.map;

        solution.inv_map = lu.matrixLU().topLeftCorner(rank, input_cols).triangularView<Eigen::Upper>();
        solution.inv_map = solution.inv_map * lu.permutationQ().inverse();

        return solutionPtr;
    }
}