/**
 * symmetrized_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/derived/lu_map_core_processor.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/representation.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "../sparse_utils.h"

namespace Moment::Tests {

    using namespace Moment::Symmetrized;

    TEST(Scenarios_Symmetry_MatrixSystem, Algebraic_Z2) {
        // Two variables, a & b
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
            std::make_unique<Algebraic::AlgebraicContext>(Algebraic::NameTable{"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        ams.generate_dictionary(2);

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));

        SymmetrizedMatrixSystem sms{amsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};

        ASSERT_EQ(&ams, &sms.base_system());


    }
}