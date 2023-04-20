/**
 * symmetrized_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/derived/symbol_table_map.h"
#include "scenarios/derived/lu_map_core_processor.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/representation.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "symbolic/symbol_table.h"

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

    TEST(Scenarios_Symmetry_MatrixSystem, Locality_CHSH) {
        // Two variables, a & b
        auto lmsPtr = std::make_shared<Locality::LocalityMatrixSystem>(
            std::make_unique<Locality::LocalityContext>(Locality::Party::MakeList(2, 2, 2))
        );
        auto& lms = *lmsPtr;
        auto& locality_context = lms.Context();
        lms.generate_dictionary(2);

        // Standard CHSH inequality symmetry
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(5, {1, 1, 0, 0, 0,
                                                0, 0, 0, 1, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 1, 0, 0,
                                                0, -1,0, 0, 0}));
        generators.emplace_back(make_sparse(5, {1, 0, 0, 0, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 0, 1, 0,
                                                0, 0, 1, 0, 0,
                                                0, 1,0, 0, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(locality_context, std::move(base_rep));

        ASSERT_EQ(group->size, 16);
        SymmetrizedMatrixSystem sms{lmsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};

        const auto& map = sms.map();
        ASSERT_EQ(lms.Symbols().size(), map.fwd_size()) << lms.Symbols(); // All symbols mapped
        EXPECT_EQ(map.inv_size(), 3); // 0, 1, 'y'
        ASSERT_EQ(sms.Symbols().size(), 3) << sms.Symbols();


        ASSERT_EQ(&lms, &sms.base_system());
    }
}