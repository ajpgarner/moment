/**
 * test_symmetrization.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include <Eigen/Sparse>

#include <memory>
#include <vector>

namespace Moment::StressTests {
    class SymmetrizedI4422 {
    private:
        std::shared_ptr<Locality::LocalityMatrixSystem> lms_ptr;

    public:
        SymmetrizedI4422();

        [[nodiscard]] std::unique_ptr<Symmetrized::Group> make_group() const;

        const Symmetrized::Representation& make_representation(Symmetrized::Group& group, size_t mm_level) const;

        size_t ensure_base_dictionary(size_t mm_level);

        [[nodiscard]] std::unique_ptr<Symmetrized::SymmetrizedMatrixSystem>
        make_symmetrized_system(std::unique_ptr<Symmetrized::Group>, size_t mm_level) const ;

        [[nodiscard]] const Locality::LocalityMatrixSystem& lms() const noexcept {
            return *lms_ptr;
        }

    private:
        [[nodiscard]] Eigen::SparseMatrix<double> make_z2_generator() const;
    };
}

