/**
 * imported_matrix_system_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix/monomial_matrix.h"
#include "symbolic/symbol_table.h"
#include "scenarios/imported/imported_matrix_system.h"

#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Imported;

    TEST(Scenarios_Imported_ImportedMatrixSystem, Empty) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        EXPECT_EQ(context.size(), 0);

        const auto& symbols = ims.Symbols();
        ASSERT_EQ(symbols.size(), 2);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_EQ(symbols[1].Id(), 1);
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Complex_TwoByTwo) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{2}, Monomial{4}, Monomial{3}, Monomial{5}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));


        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), true, false);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);

        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), Monomial{4});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), Monomial{5});

        // Check symbols
        ASSERT_EQ(symbols.size(), 6);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_FALSE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_FALSE(symbols[3].is_hermitian());
        EXPECT_FALSE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_FALSE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
        EXPECT_EQ(symbols[5].Id(), 5);
        EXPECT_FALSE(symbols[5].is_hermitian());
        EXPECT_FALSE(symbols[5].is_antihermitian());

    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, WithFactors_TwoByTwo) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{"2"}, Monomial{"0.5#3"}, Monomial{"0.5#3*"}, Monomial{"2#4"}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));


        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), true, true);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);

        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), (Monomial{3, 0.5}));
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), (Monomial{3, 0.5, true}));
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), (Monomial{4, 2.0}));

        // Check symbols
        ASSERT_EQ(symbols.size(), 5);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_TRUE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_FALSE(symbols[3].is_hermitian());
        EXPECT_FALSE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_TRUE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Symmetric_TwoByTwo) {
        ImportedMatrixSystem ims{false};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{2}, Monomial{3},
                                      Monomial{3}, Monomial{4}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));

        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), false, true);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);
        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), Monomial{4});

        // Check symbols
        ASSERT_EQ(symbols.size(), 5);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_TRUE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_TRUE(symbols[3].is_hermitian());
        EXPECT_FALSE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_TRUE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Hermitian_TwoByTwo) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{2}, Monomial{3, true}, Monomial{3}, Monomial{4}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));

        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), true, true);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);
        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), Monomial(3, true));
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), Monomial{4});

        // Check symbols
        ASSERT_EQ(symbols.size(), 5);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_TRUE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_FALSE(symbols[3].is_hermitian());
        EXPECT_FALSE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_TRUE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Hermitian_TwoByTwo_InferReal) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{2}, Monomial{3},
                                      Monomial{3}, Monomial{4}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));

        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), true, true);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);
        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), Monomial{4});

        // Check symbols
        ASSERT_EQ(symbols.size(), 5);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_TRUE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_TRUE(symbols[3].is_hermitian());
        EXPECT_FALSE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_TRUE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Hermitian_TwoByTwo_InferImaginary) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{2}, Monomial{-3}, Monomial{3}, Monomial{4}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));

        auto [index, matrix] = ims.import_matrix(std::move(raw_mat_ptr), true, true);
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);

        // Check matrix
        ASSERT_TRUE(ims[0].is_monomial());
        const auto& imported_matrix = dynamic_cast<const MonomialMatrix&>(ims[0]);
        EXPECT_EQ(&imported_matrix.symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 0), Monomial{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix(0, 1), Monomial{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 0), Monomial{-3});
        EXPECT_EQ(imported_matrix.SymbolMatrix(1, 1), Monomial{4});

        // Check symbols
        ASSERT_EQ(symbols.size(), 5);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_TRUE(symbols[0].is_hermitian());
        EXPECT_TRUE(symbols[0].is_antihermitian());
        EXPECT_EQ(symbols[1].Id(), 1);
        EXPECT_TRUE(symbols[1].is_hermitian());
        EXPECT_FALSE(symbols[1].is_antihermitian());
        EXPECT_EQ(symbols[2].Id(), 2);
        EXPECT_TRUE(symbols[2].is_hermitian());
        EXPECT_FALSE(symbols[2].is_antihermitian());
        EXPECT_EQ(symbols[3].Id(), 3);
        EXPECT_FALSE(symbols[3].is_hermitian());
        EXPECT_TRUE(symbols[3].is_antihermitian());
        EXPECT_EQ(symbols[4].Id(), 4);
        EXPECT_TRUE(symbols[4].is_hermitian());
        EXPECT_FALSE(symbols[4].is_antihermitian());
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Error_NoMomentMatrix) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        EXPECT_EQ(context.size(), 0);

        EXPECT_THROW(auto& x = ims.MomentMatrix(1), std::runtime_error);
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Error_NoLocalizingMatrix) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        EXPECT_EQ(context.size(), 0);

        LocalizingMatrixIndex lmi{1, OperatorSequence::Identity(context)};
        EXPECT_THROW(auto& y = ims.LocalizingMatrix(lmi), std::runtime_error);
    }

    TEST(Scenarios_Imported_ImportedMatrixSystem, Error_MismatchFactor) {
        ImportedMatrixSystem ims{};
        const auto &context = ims.Context();
        const auto &symbols = ims.Symbols();

        std::vector<Monomial> rawData{Monomial{"2"}, Monomial{"0.5#3"}, Monomial{"0.25#3*"}, Monomial{"2#4"}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<Monomial>>(2, std::move(rawData));

        EXPECT_THROW(ims.import_matrix(std::move(raw_mat_ptr), true, true), Imported::errors::bad_import_matrix);

    }


}
