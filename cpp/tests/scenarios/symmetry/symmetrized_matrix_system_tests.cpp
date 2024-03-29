/**
 * symmetrized_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

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

#include <array>
#include <set>
#include <sstream>
#include <stdexcept>


namespace Moment::Tests {

    namespace {
        // Find CHSH symbols:
        symbol_name_t find_symbol(const Context& context, const SymbolTable& symbols,
                                    std::initializer_list<oper_name_t> ops) {
            OperatorSequence opSeq{ops, context};
            auto symPtr = symbols.where(opSeq);
            if (symPtr != nullptr) {
                return symPtr->Id();
            }

            std::stringstream errSS;
            errSS << "Could not find symbol for " << opSeq;
            throw std::runtime_error{errSS.str()};
        };

        // Find  symbols:
        Monomial find_monomial(const Context& context, const SymbolTable& symbols,
                                    std::initializer_list<oper_name_t> ops) {
            OperatorSequence opSeq{ops, context};
            auto symPtr = symbols.where(opSeq);
            if (symPtr != nullptr) {
                return Monomial{symPtr->Id(), 1.0, symPtr.is_conjugated};
            }

            std::stringstream errSS;
            errSS << "Could not find symbol for " << opSeq;
            throw std::runtime_error{errSS.str()};
        };


        std::array<symbol_name_t, 10> get_chsh_symbol_ids(const Locality::LocalityContext& context,
                                                         const SymbolTable& symbols) {
            if (context.Parties.size() != 2) {
                throw std::runtime_error{"Two parties expected."};
            }
            const auto& alice = context.Parties[0];
            const auto& bob = context.Parties[1];
            if (alice.size() != 2) {
                throw std::runtime_error{"Alice should have two operators."};
            }
            if (bob.size() != 2) {
                throw std::runtime_error{"Bob should have two operators."};
            }

            auto a0 = find_symbol(context, symbols, {alice[0]});
            auto a1 = find_symbol(context, symbols, {alice[1]});
            auto b0 = find_symbol(context, symbols, {bob[0]});
            auto b1 = find_symbol(context, symbols, {bob[1]});

            auto a0a1 = find_symbol(context, symbols, {alice[0], alice[1]});
            auto a0b0 = find_symbol(context, symbols, {alice[0], bob[0]});
            auto a0b1 = find_symbol(context, symbols, {alice[0], bob[1]});
            auto a1b0 = find_symbol(context, symbols, {alice[1], bob[0]});
            auto a1b1 = find_symbol(context, symbols, {alice[1], bob[1]});
            auto b0b1 = find_symbol(context, symbols, {bob[0], bob[1]});

            auto output = std::array<symbol_name_t, 10>{a0, a1, b0, b1, a0a1, a0b0, a0b1, a1b0, a1b1, b0b1};

            std::set check_unique(output.begin(), output.end());
            if (check_unique.size() != 10) {
                throw std::runtime_error{"All 10 symbols should be unique."};
            }

            return output;
        }


        std::array<symbol_name_t, 5> get_algebraic_symbol_ids(const Context& context, const SymbolTable& symbols) {
            auto a = find_symbol(context, symbols, {0});
            auto b = find_symbol(context, symbols, {1});
            auto aa = find_symbol(context, symbols, {0, 0});
            auto ab = find_symbol(context, symbols, {0, 1}); // ba = ab*
            auto bb = find_symbol(context, symbols, {1, 1});

            auto output = std::array<symbol_name_t, 5>{a, b, aa, ab, bb};

            std::set check_unique(output.begin(), output.end());
            if (check_unique.size() != 5) {
                throw std::runtime_error{"All 5 symbols should be unique."};
            }
            return output;
        }

    }

    using namespace Moment::Symmetrized;

    TEST(Scenarios_Symmetry_MatrixSystem, Algebraic_Z2) {
        // Two variables, a & b
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                Algebraic::AlgebraicContext::FromNameList({"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        const auto& algebraic_symbols = ams.Symbols();
        ams.generate_dictionary(2);

        // Algebraic symbols
        const auto [a, b, aa, ab, bb] = get_algebraic_symbol_ids(context, algebraic_symbols);

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse<double>(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);

        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));
        ASSERT_EQ(group->size, 2); // I, X
        SymmetrizedMatrixSystem sms{amsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};

        ASSERT_EQ(&ams, &sms.base_system());
        const auto& sym_symbols = sms.Symbols();

        const auto& map = sms.map();
        ASSERT_EQ(algebraic_symbols.size(), map.fwd_size()) << algebraic_symbols; // All symbols mapped
        EXPECT_EQ(map.inv_size(), 5); // 0, 1, y
        ASSERT_EQ(sym_symbols.size(), 5) << sms.Symbols();
        EXPECT_TRUE(map.is_monomial_map());

        // Check inverse map
        EXPECT_EQ(map.inverse(0), Polynomial());
        EXPECT_EQ(map.inverse(1), Polynomial::Scalar(1.0));
        EXPECT_EQ(map.inverse(2), Polynomial({Monomial{a, 0.5}, Monomial{b, 0.5}}));
        EXPECT_TRUE(sym_symbols[2].is_hermitian());
        EXPECT_EQ(map.inverse(3), Polynomial({Monomial{aa, 0.5}, Monomial{bb, 0.5}}));
        EXPECT_TRUE(sym_symbols[3].is_hermitian());
        EXPECT_EQ(map.inverse(4), Polynomial({Monomial{ab, 0.5}, Monomial{ab, 0.5, true}}));
        EXPECT_TRUE(sym_symbols[4].is_hermitian());

        // Check forward map
        ASSERT_EQ(map.fwd_size(), 7);
        EXPECT_EQ(map(0), Polynomial());
        EXPECT_EQ(map(1), Polynomial::Scalar(1.0));
        EXPECT_EQ(map(a), Polynomial({Monomial{2, 1.0}}));
        EXPECT_EQ(map(b), Polynomial({Monomial{2, 1.0}}));
        EXPECT_EQ(map(aa), Polynomial({Monomial{3, 1.0}}));
        EXPECT_EQ(map(ab), Polynomial({Monomial{4, 1.0}}));
        EXPECT_EQ(map(bb), Polynomial({Monomial{3, 1.0}}));


        ASSERT_EQ(ams.size(), 0);
        ASSERT_EQ(sms.size(), 0);

        auto [mm_index, mapped_symbol_matrix] = sms.MomentMatrix.create(1);
        ASSERT_EQ(mm_index, 0); // first matrix in system.
        ASSERT_EQ(ams.size(), 1); // source system now has MM.
        const auto& source_symbol_matrix = ams[0];
        EXPECT_TRUE(source_symbol_matrix.is_monomial());

        EXPECT_FALSE(mapped_symbol_matrix.is_polynomial());
        EXPECT_TRUE(mapped_symbol_matrix.is_monomial());

        const auto& mono_sm = dynamic_cast<const  MonomialMatrix&>(mapped_symbol_matrix);
        ASSERT_EQ(mono_sm.Dimension(), 3);
        EXPECT_EQ(mono_sm.SymbolMatrix(0, 0), Monomial(1, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(0, 1), Monomial(2, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(0, 2), Monomial(2, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(1, 0), Monomial(2, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(1, 1), Monomial(3, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(1, 2), Monomial(4, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(2, 0), Monomial(2, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(2, 1), Monomial(4, 1.0));
        EXPECT_EQ(mono_sm.SymbolMatrix(2, 2), Monomial(3, 1.0));

    }

    TEST(Scenarios_Symmetry_MatrixSystem, Algebraic_Z2_PolynomialMap) {
        // Two variables, a & b
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                Algebraic::AlgebraicContext::FromNameList({"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        const auto& algebraic_symbols = ams.Symbols();
        ams.generate_dictionary(3);

        // Algebraic symbols
        const auto [a, b, aa, ab, bb] = get_algebraic_symbol_ids(context, algebraic_symbols);

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse<double>(3, {1, 0, 0,
                                                        0, 0, 1,
                                                        0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);

        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));
        ASSERT_EQ(group->size, 2); // I, X
        SymmetrizedMatrixSystem sms{amsPtr, std::move(group), 3, std::make_unique<Derived::LUMapCoreProcessor>()};

        const auto& src_factory = ams.polynomial_factory();
        const Polynomial a_plus_b = src_factory({Monomial{a, 1.0}, Monomial{b, 1.0}});
        ASSERT_EQ(a_plus_b.size(), 2);

        const auto& symmetrized_lm = sms.PolynomialLocalizingMatrix(::Moment::PolynomialLocalizingMatrixIndex{1, a_plus_b});
        ASSERT_EQ(symmetrized_lm.Dimension(), 3);
        EXPECT_EQ((symmetrized_lm.SymbolMatrix(0,0)), Polynomial(Monomial(2, 2.0))); // 'a+b' symbol

    }

    TEST(Scenarios_Symmetry_MatrixSystem, Algebraic_Z2_DerivedIndex) {
        // Two variables, a & b
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
                Algebraic::AlgebraicContext::FromNameList({"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        const auto& algebraic_symbols = ams.Symbols();
        ams.generate_dictionary(3);

        // Algebraic symbols
        const auto [a, b, aa, ab, bb] = get_algebraic_symbol_ids(context, algebraic_symbols);

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse<double>(3, {1, 0, 0,
                                                        0, 0, 1,
                                                        0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);

        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));
        ASSERT_EQ(group->size, 2); // I, X
        SymmetrizedMatrixSystem sms{amsPtr, std::move(group), 3, std::make_unique<Derived::LUMapCoreProcessor>()};

        const auto& src_factory = ams.polynomial_factory();
        const Polynomial a_plus_b = src_factory({Monomial{a, 1.0}, Monomial{b, 1.0}});
        ASSERT_EQ(a_plus_b.size(), 2);
        const ::Moment::PolynomialLocalizingMatrixIndex plm_index{1, a_plus_b};
        const auto& src_lm = ams.PolynomialLocalizingMatrix(plm_index);
        ASSERT_EQ(src_lm.Dimension(), 3);
        auto src_mat_offset = ams.PolynomialLocalizingMatrix.find_index(plm_index);
        ASSERT_EQ(src_mat_offset, 2); // a = 0, b = 1, a + b = 2

        const auto& symmetrized_lm = sms.DerivedMatrices(src_mat_offset);
        ASSERT_TRUE(symmetrized_lm.is_polynomial());
        const auto& slm_as_poly = dynamic_cast<const PolynomialMatrix&>(symmetrized_lm);
        ASSERT_EQ(symmetrized_lm.Dimension(), 3);
        EXPECT_EQ((slm_as_poly.SymbolMatrix(0,0)), Polynomial(Monomial(2, 2.0))); // 'a+b' symbol
    }

    TEST(Scenarios_Symmetry_MatrixSystem, Locality_CHSH) {
        // Two variables, a & b
        auto lmsPtr = std::make_shared<Locality::LocalityMatrixSystem>(
            std::make_unique<Locality::LocalityContext>(Locality::Party::MakeList(2, 2, 2))
        );
        auto& lms = *lmsPtr;
        auto& locality_context = lms.localityContext;
        auto& locality_symbols = lms.Symbols();
        lms.generate_dictionary(2);

        // Get CHSH symbols
        const auto [a0, a1, b0, b1, a0a1, a0b0, a0b1, a1b0, a1b1, b0b1] =
                get_chsh_symbol_ids(locality_context, locality_symbols);


        // Standard CHSH inequality symmetry
        std::vector<Eigen::SparseMatrix<double>> generators;
     generators.emplace_back(make_sparse<double>(5, {1, 0, 1, 0, 0,
                                             0, 1, 0, 0, 0,
                                             0, 0,-1, 0, 0,
                                             0, 0, 0, 0, 1,
                                             0, 0, 0, 1, 0}));
        generators.emplace_back(make_sparse<double>(5, {1, 0, 0, 0, 0,
                                                0, 0, 0, 1, 0,
                                                0, 0, 0, 0, 1,
                                                0, 1, 0, 0, 0,
                                                0, 0, 1, 0, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(locality_context, std::move(base_rep));

        ASSERT_EQ(group->size, 16);
        SymmetrizedMatrixSystem sms{lmsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};
        ASSERT_EQ(&lms, &sms.base_system());
        const auto& sym_symbols = sms.Symbols();

        const auto& map = sms.map();
        ASSERT_EQ(locality_symbols.size(), map.fwd_size()) << lms.Symbols(); // All symbols mapped
        EXPECT_EQ(map.inv_size(), 3); // 0, 1, y
        ASSERT_EQ(sym_symbols.size(), 3) << sms.Symbols();
        EXPECT_FALSE(map.is_monomial_map());

        // Check inverse map
        EXPECT_EQ(map.inverse(0), Polynomial());
        EXPECT_EQ(map.inverse(1), Polynomial::Scalar(1.0));
        Polynomial expected_new_symbol{Monomial{a0, -0.25},
                                       Monomial{b0, -0.25},
                                       Monomial{a0b0, +0.25},
                                       Monomial{a0b1, +0.25},
                                       Monomial{a1b0, +0.25},
                                       Monomial{a1b1, -0.25}};
        EXPECT_EQ(map.inverse(2), expected_new_symbol);

        // Check forward map
        ASSERT_EQ(map.fwd_size(), 12);
        EXPECT_EQ(map(0), Polynomial());
        EXPECT_EQ(map(1), Polynomial::Scalar(1.0));
        EXPECT_EQ(map(a0), Polynomial::Scalar(0.5));
        EXPECT_EQ(map(a1), Polynomial::Scalar(0.5));
        EXPECT_EQ(map(b0), Polynomial::Scalar(0.5));
        EXPECT_EQ(map(b1), Polynomial::Scalar(0.5));
        EXPECT_EQ(map(a0a1), Polynomial::Scalar(0.25));
        EXPECT_EQ(map(b0b1), Polynomial::Scalar(0.25));
        EXPECT_EQ(map(a0b0), Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)}));
        EXPECT_EQ(map(a0b1), Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)}));
        EXPECT_EQ(map(a1b0), Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)}));
        EXPECT_EQ(map(a1b1), Polynomial({Monomial(1, 0.125), Monomial(2, -1.0)}));

        // Check on CHSH inequality
        Polynomial chsh_ineq{Monomial{1, 2.0}, Monomial{a0, -4.0}, Monomial{b0, -4.0},
                             Monomial{a0b0, 4.0}, Monomial{a0b1, 4.0},
                             Monomial{a1b0, 4.0}, Monomial{a1b1, -4.0}};
        Polynomial mapped_chsh{Monomial{1, 2.0}, Monomial{2, 16.0}};

        EXPECT_EQ(map(chsh_ineq), mapped_chsh);

        // Make moment matrix.

        ASSERT_EQ(lms.size(), 0);
        ASSERT_EQ(sms.size(), 0);

        auto [mm_index, mapped_symbol_matrix] = sms.MomentMatrix.create(1);
        ASSERT_EQ(mm_index, 0); // first matrix in system.
        ASSERT_EQ(lms.size(), 1); // source system now has MM.
        const auto& source_symbol_matrix = lms[0];
        EXPECT_TRUE(source_symbol_matrix.is_monomial());

        EXPECT_TRUE(mapped_symbol_matrix.is_polynomial());
        EXPECT_FALSE(mapped_symbol_matrix.is_monomial());

        const auto& poly_sm = dynamic_cast<const PolynomialMatrix&>(mapped_symbol_matrix);
        ASSERT_EQ(poly_sm.Dimension(), 5);
        EXPECT_EQ(poly_sm.SymbolMatrix(0, 0), Polynomial::Scalar(1.0));
        EXPECT_EQ(poly_sm.SymbolMatrix(0, 1), Polynomial::Scalar(0.5));
        EXPECT_EQ(poly_sm.SymbolMatrix(0, 2), Polynomial::Scalar(0.5));
        EXPECT_EQ(poly_sm.SymbolMatrix(0, 3), Polynomial::Scalar(0.5));
        EXPECT_EQ(poly_sm.SymbolMatrix(0, 4), Polynomial::Scalar(0.5));

        EXPECT_EQ(poly_sm.SymbolMatrix(1, 0), Polynomial::Scalar(0.5));
        EXPECT_EQ(poly_sm.SymbolMatrix(1, 1), Polynomial::Scalar(0.5)); // a0^2 -> a0 -> 0.5
        EXPECT_EQ(poly_sm.SymbolMatrix(1, 2), Polynomial::Scalar(0.25)); // a0a1 -> 0.25
        EXPECT_EQ(poly_sm.SymbolMatrix(1, 3),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a0b0 -> 0.375 + y
        EXPECT_EQ(poly_sm.SymbolMatrix(1, 4),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a0b1 -> 0.125 - y

        EXPECT_EQ(poly_sm.SymbolMatrix(2, 0), Polynomial::Scalar(0.5)); // a1
        EXPECT_EQ(poly_sm.SymbolMatrix(2, 1), Polynomial::Scalar(0.25)); // a1a0
        EXPECT_EQ(poly_sm.SymbolMatrix(2, 2), Polynomial::Scalar(0.5)); // a1^2 -> a1 -> 0.5
        EXPECT_EQ(poly_sm.SymbolMatrix(2, 3),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a1b0 -> 0.375 + y
        EXPECT_EQ(poly_sm.SymbolMatrix(2, 4),
                  Polynomial({Monomial(1, 0.125), Monomial(2, -1.0)})); // a1b1 -> 0.125 - y

        EXPECT_EQ(poly_sm.SymbolMatrix(3, 0), Polynomial::Scalar(0.5)); // b0 -> 0.5
        EXPECT_EQ(poly_sm.SymbolMatrix(3, 1),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a0b0 -> 0.375 + y
        EXPECT_EQ(poly_sm.SymbolMatrix(3, 2),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a1b0 -> 0.375 + y
        EXPECT_EQ(poly_sm.SymbolMatrix(3, 3), Polynomial::Scalar(0.5));  // b0^2 -> b0 -> 0.5
        EXPECT_EQ(poly_sm.SymbolMatrix(3, 4), Polynomial::Scalar(0.25)); // b0b1 -> 0.25

        EXPECT_EQ(poly_sm.SymbolMatrix(4, 0), Polynomial::Scalar(0.5)); // b1 -> 0.5
        EXPECT_EQ(poly_sm.SymbolMatrix(4, 1),
                  Polynomial({Monomial(1, 0.375), Monomial(2, 1.0)})); // a0b1 -> 0.375 - y
        EXPECT_EQ(poly_sm.SymbolMatrix(4, 2),
                  Polynomial({Monomial(1, 0.125), Monomial(2, -1.0)})); // a1b1 -> 0.125 - y
        EXPECT_EQ(poly_sm.SymbolMatrix(4, 3), Polynomial::Scalar(0.25));  // b1b0 -> 0.25
        EXPECT_EQ(poly_sm.SymbolMatrix(4, 4), Polynomial::Scalar(0.5)); // b1^2 -> b1 -> 0.25
    }


    TEST(Scenarios_Symmetry_MatrixSystem, Locality_CHSH_Level3) {
        // Two variables, a & b
        auto lmsPtr = std::make_shared<Locality::LocalityMatrixSystem>(
                std::make_unique<Locality::LocalityContext>(Locality::Party::MakeList(2, 2, 2))
        );
        auto &lms = *lmsPtr;
        auto &locality_context = lms.localityContext;
        auto &locality_symbols = lms.Symbols();
        lms.generate_dictionary(6);

        // Get CHSH symbols
        const auto [a0, a1, b0, b1, a0a1, a0b0, a0b1, a1b0, a1b1, b0b1] =
                get_chsh_symbol_ids(locality_context, locality_symbols);


        // Standard CHSH inequality symmetry
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse<double>(5, {1, 0, 1, 0, 0,
                                                0, 1, 0, 0, 0,
                                                0, 0, -1, 0, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 0, 1, 0}));
        generators.emplace_back(make_sparse<double>(5, {1, 0, 0, 0, 0,
                                                0, 0, 0, 1, 0,
                                                0, 0, 0, 0, 1,
                                                0, 1, 0, 0, 0,
                                                0, 0, 1, 0, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(locality_context, std::move(base_rep));

        ASSERT_EQ(group->size, 16);
        SymmetrizedMatrixSystem sms{lmsPtr, std::move(group), 6, std::make_unique<Derived::LUMapCoreProcessor>()};
        ASSERT_EQ(&lms, &sms.base_system());
        const auto& sym_symbols = sms.Symbols();

        const auto& map = sms.map();
        ASSERT_EQ(locality_symbols.size(), map.fwd_size()) << lms.Symbols(); // All symbols mapped
        EXPECT_FALSE(map.is_monomial_map());

        // Check inverse map
        EXPECT_EQ(map.inverse(0), Polynomial());
        EXPECT_EQ(map.inverse(1), Polynomial::Scalar(1.0));


    }
}