/**
 * kronecker_power.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "kronecker_power.h"

#include <unsupported/Eigen/KroneckerProduct>
#include <cassert>

namespace Moment {

    namespace {
        inline Eigen::SparseMatrix<double> id1() {
            Eigen::SparseMatrix<double> id{1,1};
            id.setIdentity();
            return id;
        }
    }

    Eigen::SparseMatrix<double> kronecker_power(const Eigen::SparseMatrix<double>& base, int power) {

        // Handle trivial cases
        if (power == 1) {
            return base;
        } else if (power == 0) {
            return id1();
        }
        assert(power>0);

        // Handle first bit:
        Eigen::SparseMatrix<double> output = ((power & 1)==1) ? base : id1();
        power = power >> 1;

        // Accumulate powers of two...
        Eigen::SparseMatrix<double> pow2 = base;
        while (0 != power) {
            pow2 = Eigen::kroneckerProduct(pow2, pow2).eval();
            if ((power & 1) == 1) {
                output = Eigen::kroneckerProduct(output, pow2).eval();
            }

            power = power >> 1;
        }

        return output;
    }


}