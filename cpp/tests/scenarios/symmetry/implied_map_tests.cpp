/**
 * implied_map_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/implied_map.h"
#include "scenarios/symmetrized/representation.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "sparse_utils.h"

namespace Moment::Tests {

    using namespace Moment::Symmetrized;

    TEST(Scenarios_Symmetry_ImpliedMap, BasicLevel1) {
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                std::make_unique<Algebraic::AlgebraicContext>(Algebraic::NameTable{"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        ams.generate_dictionary(1);
        EXPECT_EQ(ams.Symbols().size(), 4); // 0, 1, a, b

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));

        SymmetrizedMatrixSystem sms{amsPtr, std::move(group)};

        //ImpliedMap map{sms, sms.group().representation(1)};



    }
}