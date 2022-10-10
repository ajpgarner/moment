/**
 * npa_matrix_Tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/matrix/moment_matrix.h"
#include "operators/matrix/matrix_system.h"
#include "operators/locality/locality_context.h"
#include "operators/locality/locality_matrix_system.h"

#include "compare_os_matrix.h"

namespace NPATK::Tests {
    namespace {

        struct unique_seq_brace_ref {
            OperatorSequence fwd;
            OperatorSequence rev;
            bool herm;
        };

        void compare_unique_sequences(MomentMatrix &theMM, std::initializer_list<unique_seq_brace_ref> reference) {
            ASSERT_EQ(theMM.Symbols.size(), 2 + reference.size());

            // 0 is always zero
            auto iter = theMM.Symbols.begin();
            ASSERT_NE(iter, theMM.Symbols.end()) << " Level = " << theMM.Level();
            EXPECT_EQ(&(*iter), &theMM.Symbols[0]) << " Level = " << theMM.Level();
            EXPECT_EQ(theMM.Symbols[0].sequence(), OperatorSequence::Zero(theMM.context))
                << " Level = " << theMM.Level();
            EXPECT_EQ(theMM.Symbols[0].sequence_conj(), OperatorSequence::Zero(theMM.context))
                << " Level = " << theMM.Level();
            EXPECT_TRUE(theMM.Symbols[0].is_hermitian()) << " Level = " << theMM.Level();
            ++iter;

            // 1 is always ID
            ASSERT_NE(iter, theMM.Symbols.end()) << " Level = " << theMM.Level();
            EXPECT_EQ(&(*iter), &theMM.Symbols[1]) << " Level = " << theMM.Level();
            EXPECT_EQ(theMM.Symbols[1].sequence(), OperatorSequence::Identity(theMM.context))
                << " Level = " << theMM.Level();
            EXPECT_EQ(theMM.Symbols[1].sequence_conj(), OperatorSequence::Identity(theMM.context))
                << " Level = " << theMM.Level();
            EXPECT_TRUE(theMM.Symbols[1].is_hermitian())  << " Level = " << theMM.Level();
            ++iter;

            size_t index = 2;
            for (const auto& ref_seq : reference) {
                ASSERT_NE(iter, theMM.Symbols.end()) << " Level = " << theMM.Level() << ", index = " << index;
                EXPECT_EQ(&(*iter), &theMM.Symbols[index]) << " Level = " << theMM.Level()
                    << ", index = " << index;
                EXPECT_EQ(iter->sequence(), ref_seq.fwd) << " Level = " << theMM.Level() << ", index = " << index;
                EXPECT_EQ(iter->sequence_conj(), ref_seq.rev) << " Level = " << theMM.Level() << ", index = " << index;
                EXPECT_EQ(iter->is_hermitian(), ref_seq.herm) << " Level = " << theMM.Level() << ", index = " << index;
                ++index;
                ++iter;
            }

            EXPECT_EQ(index, 2 + reference.size()) << " Level = " << theMM.Level();
            EXPECT_EQ(iter, theMM.Symbols.end()) << " Level = " << theMM.Level();
        }

        void compare_symbol_matrix(MomentMatrix &theMM, size_t dimension,
                                   const std::vector<SymbolExpression>& reference) {
            ASSERT_EQ(theMM.SymbolMatrix.Dimension(), dimension);

            size_t row = 0;
            size_t col = 0;
            for (const auto &ref_symbol: reference) {
                ASSERT_LT(row, dimension) << " Level = " << theMM.Level() << ", row = " << row << ", col = " << col;
                ASSERT_LT(col, dimension) << " Level = " << theMM.Level() << ", row = " << row << ", col = " << col;

                const auto &actual_symbol = theMM.SymbolMatrix[row][col];
                EXPECT_EQ(actual_symbol, ref_symbol)
                                    << " Level = " << theMM.Level() << ", row = " << row << ", col = " << col;
                ++col;
                if (col >= dimension) {
                    col = 0;
                    ++row;
                }
            }
            EXPECT_EQ(col, 0) << " Level = " << theMM.Level();
            EXPECT_EQ(row, dimension) << " Level = " << theMM.Level();

        }

        void compare_symbol_matrix(MomentMatrix &theMM, size_t dimension,
                               std::initializer_list<std::string> reference) {
            std::vector<SymbolExpression> txReference;
            txReference.reserve(reference.size());
            for (const auto& str : reference) {
                txReference.emplace_back(str);
            }
            compare_symbol_matrix(theMM, dimension, txReference);
        }
    }

    TEST(MomentMatrix, Empty) {
        MatrixSystem system{std::make_unique<Context>()}; // No parties, no symbols
        auto& context = system.Context();
        ASSERT_EQ(context.size(), 0);

        auto& matLevel0 = system.CreateMomentMatrix(0);

        EXPECT_EQ(matLevel0.Level(), 0);
        compare_mm_os_matrix(matLevel0, 1, {OperatorSequence::Identity(context)});
        compare_unique_sequences(matLevel0, {});
        compare_symbol_matrix(matLevel0, 1, {"1"});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        EXPECT_EQ(matLevel1.Level(), 1);
        compare_mm_os_matrix(matLevel1, 1, {OperatorSequence::Identity(context)});
        compare_unique_sequences(matLevel1, {});
        compare_symbol_matrix(matLevel1, 1, {"1"});

        auto& matLevel5 = system.CreateMomentMatrix(5);
        EXPECT_EQ(matLevel5.Level(), 5);
        compare_mm_os_matrix(matLevel5, 1, {OperatorSequence::Identity(context)});
        compare_unique_sequences(matLevel5, {});
        compare_symbol_matrix(matLevel1, 1, {"1"});
    }

    TEST(MomentMatrix, OpSeq_OneElem) {
        MatrixSystem system{std::make_unique<Context>(1)}; // One symbol
        auto& context = system.Context();

        ASSERT_EQ(context.size(), 1);
        const auto& theOp = context[0];

        auto& matLevel0 = system.CreateMomentMatrix(0);
        EXPECT_EQ(matLevel0.Level(), 0);
        compare_mm_os_matrix(matLevel0, 1, {OperatorSequence::Identity(context)});


        auto& matLevel1 = system.CreateMomentMatrix(1);
        EXPECT_EQ(matLevel1.Level(), 1);
        compare_mm_os_matrix(matLevel1, 2, {OperatorSequence::Identity(context),
                                         OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context)});


        auto& matLevel2 = system.CreateMomentMatrix(2);
        EXPECT_EQ(matLevel2.Level(), 2);
        compare_mm_os_matrix(matLevel2, 3, {OperatorSequence::Identity(context),
                                         OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp, theOp}, context)});
    }

    TEST(MomentMatrix, OpSeq_TwoElem) {
        MatrixSystem system{std::make_unique<Context>(2)}; // Two elements
        const auto& context = system.Context();
        const auto& alice = context;

        ASSERT_EQ(alice.size(), 2);

        auto& matLevel0 = system.CreateMomentMatrix(0);

        compare_mm_os_matrix(matLevel0, 1, {OperatorSequence::Identity(context)});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_mm_os_matrix(matLevel1, 3, {OperatorSequence::Identity(context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[1]}, context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1]}, context)});

        auto& matLevel2 = system.CreateMomentMatrix(2);
        compare_mm_os_matrix(matLevel2, 7, {OperatorSequence::Identity(context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[1]}, context),
                                         OperatorSequence({alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[0], alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[0], alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[0], alice[1], alice[1] , alice[1]}, context),

                                         OperatorSequence({alice[1], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[0], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[0], alice[1]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[1], alice[0]}, context),
                                         OperatorSequence({alice[1], alice[1], alice[1] , alice[1]}, context)}

                 );
    };

    TEST(MomentMatrix, OpSeq_2Party1Opers) {
        // Two parties, each with one operator
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        auto& context = system.localityContext;

        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        auto& matLevel0 = system.CreateMomentMatrix(0);

        compare_mm_os_matrix(matLevel0, 1, {OperatorSequence::Identity(context)});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_mm_os_matrix(matLevel1, 3, {OperatorSequence::Identity(context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({bob[0]}, context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({bob[0]}, context)});

        auto& matLevel2 = system.CreateMomentMatrix(2);
        compare_mm_os_matrix(matLevel2, 4, {OperatorSequence::Identity(context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),

                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),

                                         OperatorSequence({bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),

                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context),
                                         OperatorSequence({alice[0], bob[0]}, context)});
    }


    TEST(MomentMatrix, OpSeq_223) {
         // Two party, two mmts, three outcomes.
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 3))};
        auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        EXPECT_EQ(context.size(), 8);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 4);
        ASSERT_EQ(bob.size(), 4);
        const auto& a0 = alice[0];
        const auto& a1 = alice[1];
        const auto& b0 = alice[2];
        const auto& b1 = alice[3];

        const auto& x0 = bob[0];
        const auto& x1 = bob[1];
        const auto& y0 = bob[2];
        const auto& y1 = bob[3];

        auto& matLevel0 = system.CreateMomentMatrix(0);
        compare_mm_os_matrix(matLevel0, 1, {OperatorSequence::Identity(context)});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_mm_os_matrix(matLevel1, 9, {OperatorSequence::Identity(context),
                                         OperatorSequence({a0}, context),
                                         OperatorSequence({a1}, context),
                                         OperatorSequence({b0}, context),
                                         OperatorSequence({b1}, context),
                                         OperatorSequence({x0}, context),
                                         OperatorSequence({x1}, context),
                                         OperatorSequence({y0}, context),
                                         OperatorSequence({y1}, context),

                                         OperatorSequence({a0}, context),
                                         OperatorSequence({a0}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({a0, b0}, context),
                                         OperatorSequence({a0, b1}, context),
                                         OperatorSequence({a0, x0}, context),
                                         OperatorSequence({a0, x1}, context),
                                         OperatorSequence({a0, y0}, context),
                                         OperatorSequence({a0, y1}, context),

                                         OperatorSequence({a1}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({a1}, context),
                                         OperatorSequence({a1, b0}, context),
                                         OperatorSequence({a1, b1}, context),
                                         OperatorSequence({a1, x0}, context),
                                         OperatorSequence({a1, x1}, context),
                                         OperatorSequence({a1, y0}, context),
                                         OperatorSequence({a1, y1}, context),

                                         OperatorSequence({b0}, context),
                                         OperatorSequence({b0, a0}, context),
                                         OperatorSequence({b0, a1}, context),
                                         OperatorSequence({b0}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({b0, x0}, context),
                                         OperatorSequence({b0, x1}, context),
                                         OperatorSequence({b0, y0}, context),
                                         OperatorSequence({b0, y1}, context),

                                         OperatorSequence({b1}, context),
                                         OperatorSequence({b1, a0}, context),
                                         OperatorSequence({b1, a1}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({b1}, context),
                                         OperatorSequence({b1, x0}, context),
                                         OperatorSequence({b1, x1}, context),
                                         OperatorSequence({b1, y0}, context),
                                         OperatorSequence({b1, y1}, context),

                                         OperatorSequence({x0}, context),
                                         OperatorSequence({a0, x0}, context),
                                         OperatorSequence({a1, x0}, context),
                                         OperatorSequence({b0, x0}, context),
                                         OperatorSequence({b1, x0}, context),
                                         OperatorSequence({x0}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({x0, y0}, context),
                                         OperatorSequence({x0, y1}, context),

                                         OperatorSequence({x1}, context),
                                         OperatorSequence({a0, x1}, context),
                                         OperatorSequence({a1, x1}, context),
                                         OperatorSequence({b0, x1}, context),
                                         OperatorSequence({b1, x1}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({x1}, context),
                                         OperatorSequence({x1, y0}, context),
                                         OperatorSequence({x1, y1}, context),

                                         OperatorSequence({y0}, context),
                                         OperatorSequence({a0, y0}, context),
                                         OperatorSequence({a1, y0}, context),
                                         OperatorSequence({b0, y0}, context),
                                         OperatorSequence({b1, y0}, context),
                                         OperatorSequence({y0, x0}, context),
                                         OperatorSequence({y0, x1}, context),
                                         OperatorSequence({y0}, context),
                                         OperatorSequence::Zero(context),

                                         OperatorSequence({y1}, context),
                                         OperatorSequence({a0, y1}, context),
                                         OperatorSequence({a1, y1}, context),
                                         OperatorSequence({b0, y1}, context),
                                         OperatorSequence({b1, y1}, context),
                                         OperatorSequence({y1, x0}, context),
                                         OperatorSequence({y1, x1}, context),
                                         OperatorSequence::Zero(context),
                                         OperatorSequence({y1}, context)
        });

    }

    TEST(MomentMatrix, Unique_OneElem) {
        // One party, one symbol
        MatrixSystem system{std::make_unique<Context>(1)};
        auto& context = system.Context();

        ASSERT_EQ(context.size(), 1);
        const auto& alice = context;


        auto& matLevel0 = system.CreateMomentMatrix(0);
        compare_unique_sequences(matLevel0, {});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_unique_sequences(matLevel1, {{OperatorSequence({alice[0]}, context),
                                                     OperatorSequence({alice[0]}, context), true},
                                             {OperatorSequence({alice[0], alice[0]}, context),
                                                     OperatorSequence({alice[0], alice[0]}, context), true}});

        auto& matLevel2 = system.CreateMomentMatrix(2);
        compare_unique_sequences(matLevel2,
                                 {{OperatorSequence({alice[0]}, context),
                                          OperatorSequence({alice[0]}, context), true},
                                  {OperatorSequence({alice[0], alice[0]}, context),
                                          OperatorSequence({alice[0], alice[0]}, context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0]}, context),
                                          OperatorSequence({alice[0], alice[0], alice[0]}, context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, context),
                                          OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, context), true}});
    }

    TEST(MomentMatrix, Unique_2Party1Opers) {
        // Two parties, each with one operator
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 1);
        ASSERT_EQ(bob.size(), 1);

        auto& matLevel0 = system.CreateMomentMatrix(0);

        compare_unique_sequences(matLevel0, {});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_unique_sequences(matLevel1,
                                 {{OperatorSequence({alice[0]}, context),
                                   OperatorSequence({alice[0]}, context), true},
                                  {OperatorSequence({bob[0]}, context),
                                   OperatorSequence({bob[0]}, context), true},
                                  {OperatorSequence({alice[0], bob[0]}, context),
                                   OperatorSequence({alice[0], bob[0]}, context), true}});

        auto& matLevel2 = system.CreateMomentMatrix(2);
        compare_unique_sequences(matLevel2,
                                 {{OperatorSequence({alice[0]}, context),
                                   OperatorSequence({alice[0]}, context), true},
                                  {OperatorSequence({bob[0]}, context),
                                   OperatorSequence({bob[0]}, context), true},
                                  {OperatorSequence({alice[0], bob[0]}, context),
                                   OperatorSequence({alice[0], bob[0]}, context), true}});
    }

    TEST(MomentMatrix, Unique_1Party2Opers_L0) {
        MatrixSystem system{std::make_unique<Context>(2)}; // Two symbols
        const auto& context = system.Context();
        ASSERT_EQ(context.size(), 2);
        const auto &alice = context;

        auto &matLevel0 = system.CreateMomentMatrix(0);
        compare_unique_sequences(matLevel0, {});

    }

    TEST(MomentMatrix, Unique_1Party2Opers_L1) {
        MatrixSystem system{std::make_unique<Context>(2)}; // Two symbols
        const auto& context = system.Context();
        ASSERT_EQ(context.size(), 2);
        const auto &alice = context;
        ASSERT_EQ(alice.size(), 2);
        auto &matLevel1 = system.CreateMomentMatrix(1);

        compare_unique_sequences(matLevel1, {{OperatorSequence({alice[0]}, context),
                                                     OperatorSequence({alice[0]}, context),           true},
                                             {OperatorSequence({alice[1]}, context),
                                                     OperatorSequence({alice[1]}, context),           true},
                                             {OperatorSequence({alice[0], alice[0]}, context),
                                                     OperatorSequence({alice[0], alice[0]}, context), true},
                                             {OperatorSequence({alice[0], alice[1]}, context),
                                                     OperatorSequence({alice[1], alice[0]}, context), false},
                                             {OperatorSequence({alice[1], alice[1]}, context),
                                                     OperatorSequence({alice[1], alice[1]}, context), true}});

    }

    TEST(MomentMatrix, Unique_1Party2Opers_L2) {
        MatrixSystem system{std::make_unique<Context>(2)}; // One party, two symbols
        const auto& context = system.Context();
        ASSERT_EQ(context.size(), 2);
        const auto &alice = context;
        auto& matLevel2 = system.CreateMomentMatrix(2);

        compare_unique_sequences(matLevel2, {
                {OperatorSequence({alice[0]}, context), // 2
                        OperatorSequence({alice[0]}, context), true},
                {OperatorSequence({alice[1]}, context),
                        OperatorSequence({alice[1]}, context), true},

                {OperatorSequence({alice[0], alice[0]}, context), // 4
                        OperatorSequence({alice[0], alice[0]}, context), true},
                {OperatorSequence({alice[0], alice[1]}, context),
                        OperatorSequence({alice[1], alice[0]}, context), false},
                {OperatorSequence({alice[1], alice[1]}, context),
                        OperatorSequence({alice[1], alice[1]}, context), true},

                {OperatorSequence({alice[0], alice[0], alice[0]}, context), // 7
                        OperatorSequence({alice[0], alice[0], alice[0]}, context), true},
                {OperatorSequence({alice[0], alice[0], alice[1]}, context),
                        OperatorSequence({alice[1], alice[0], alice[0]}, context), false},
                {OperatorSequence({alice[0], alice[1], alice[0]}, context),
                        OperatorSequence({alice[0], alice[1], alice[0]}, context), true},
                {OperatorSequence({alice[0], alice[1], alice[1]}, context),
                        OperatorSequence({alice[1], alice[1], alice[0]}, context), false},
                {OperatorSequence({alice[1], alice[0], alice[1]}, context),
                        OperatorSequence({alice[1], alice[0], alice[1]}, context), true},
                {OperatorSequence({alice[1], alice[1], alice[1]}, context),
                        OperatorSequence({alice[1], alice[1], alice[1]}, context), true},

                {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, context), // 13
                        OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, context), true},
                {OperatorSequence({alice[0], alice[0], alice[0], alice[1]}, context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[0]}, context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[0]}, context),
                        OperatorSequence({alice[0], alice[1], alice[0], alice[0]}, context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[1]}, context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[0]}, context), false},
                {OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , context), true},
                {OperatorSequence({alice[0], alice[1], alice[0], alice[1]} , context),
                        OperatorSequence({alice[1], alice[0], alice[1], alice[0]} , context), false},
                {OperatorSequence({alice[1], alice[0], alice[1], alice[1]} , context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[1]} , context), false},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, context),
                        OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, context), true},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[1]} , context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[0]} , context), false},
                {OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , context), true}
        });
    }

    TEST(MomentMatrix, Where_1Party2Opers) {
        MatrixSystem system{std::make_unique<Context>(2)}; // Two symbols
        auto& context = system.Context();
        ASSERT_EQ(context.size(), 2);
        const auto& alice = context;

        auto& matLevel2 = system.CreateMomentMatrix(2);

        auto ptr_a0a0a0a0 = matLevel2.Symbols.where(OperatorSequence{{alice[0], alice[0], alice[0], alice[0]},
                                                                     context});
        ASSERT_NE(ptr_a0a0a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a0a0->sequence(), (OperatorSequence{{alice[0], alice[0], alice[0], alice[0]}, context}));

        auto ptr_a0a0a1a1 = matLevel2.Symbols.where(OperatorSequence{{alice[0], alice[0], alice[1], alice[1]},
                                                                     context});
        auto ptr_a1a1a0a0 = matLevel2.Symbols.where(OperatorSequence{{alice[1], alice[1], alice[0], alice[0]},
                                                                     context});
        ASSERT_NE(ptr_a0a0a1a1, nullptr);
        ASSERT_NE(ptr_a1a1a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a1a1, ptr_a1a1a0a0);

        EXPECT_EQ(ptr_a0a0a1a1->sequence(), (OperatorSequence{{alice[0], alice[0], alice[1], alice[1]},
                                                              context}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence(), (OperatorSequence{{alice[0], alice[0], alice[1], alice[1]},
                                                              context}));
        EXPECT_EQ(ptr_a0a0a1a1->sequence_conj(), (OperatorSequence{{alice[1], alice[1], alice[0], alice[0]},
                                                                   context}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence_conj(), (OperatorSequence{{alice[1], alice[1], alice[0], alice[0]},
                                                                   context}));

        auto ptr_a0a0a0a0a0 = matLevel2.Symbols.where(
                OperatorSequence{{alice[0], alice[0], alice[0], alice[0], alice[0]}, context});
        EXPECT_EQ(ptr_a0a0a0a0a0, nullptr);
    }

    TEST(MomentMatrix, Symbol_OneElem) {
        MatrixSystem system{std::make_unique<Context>(1)}; // One party, one symbol
        auto& context = system.Context();

        auto& matLevel0 = system.CreateMomentMatrix(0);
        compare_symbol_matrix(matLevel0, 1, {"1"});

        auto& matLevel1 = system.CreateMomentMatrix(1); // id, a, a^2
        compare_symbol_matrix(matLevel1, 2, {"1", "2",
                                             "2", "3"});

        auto& matLevel2 = system.CreateMomentMatrix(2); // id, a, a^2, a^3, a^4
        compare_symbol_matrix(matLevel2, 3, {"1", "2", "3",
                                             "2", "3", "4",
                                             "3", "4", "5"});
    }

    TEST(MomentMatrix, Symbol_1Party2Opers) {
        MatrixSystem system{std::make_unique<Context>(2)}; // One party, two symbols
        auto& context = system.Context();

        auto& matLevel0 = system.CreateMomentMatrix(0);
        compare_symbol_matrix(matLevel0, 1, {"1"});

        auto& matLevel1 = system.CreateMomentMatrix(1); // x, 0, 1???
        compare_symbol_matrix(matLevel1, 3, {"1",  "2", "3",
                                             "2",  "4", "5",
                                             "3", "5*", "6"});

        auto& matLevel2 = system.CreateMomentMatrix(2);
        compare_symbol_matrix(matLevel2, 7, // Remember symbol order is from hash function...
                              {"1",  "2",   "3",   "4",   "5",  "5*", "6",  // x, 0,  1,  00,  01,  10,  11
                               "2",  "4",   "5",   "7",   "8",  "9",  "10", // 0, 00, 01, 000, 001, 010, 011
                               "3",  "5*",  "6",   "8*",  "11", "10*", "12", // 1, 10, 11, 100, 101, 110, 111
                               "4",  "7",   "8",   "13",  "14", "15", "16", // 001, 000, 001, 0000, 0001, 0010, 0011
                               "5*", "8*",  "11",  "14*", "17", "18*", "19", // 10, 100, 101, 1000, 1001, 1010, 1011
                               "5",  "9",   "10",  "15*", "18", "20", "21", // 01, 010, 011, 0100, 0101, 0110, 0111
                               "6",  "10*", "12",  "16*", "19*","21*","22" // 11, 110, 111, 1100, 1101, 1110, 1111
        });
    };

    TEST(MomentMatrix, Symbol_2Party1Opers) {
        // Two parties, each with one operator
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))}; // Two party, one operator each
        auto& context = system.Context();

        auto& matLevel0 = system.CreateMomentMatrix(0);
        compare_symbol_matrix(matLevel0, 1, {"1"});

        auto& matLevel1 = system.CreateMomentMatrix(1);
        compare_symbol_matrix(matLevel1, 3, {"1", "2", "3",   // 1, a, b
                                             "2", "2", "4",   // a, aa, ab
                                             "3", "4", "3"}); // b, ab, b

        auto& matLevel2 = system.CreateMomentMatrix(2); // order of unique symbols: 1, a, b, ab
        compare_symbol_matrix(matLevel2, 4, {"1", "2", "3", "4",  // 1, a, b, ab
                                             "2", "2", "4", "4",  // a, a, ab, ab
                                             "3", "4", "3", "4",  // b, ab, b, ab
                                             "4", "4", "4", "4"});// ab, ab, ab, ab
    }
}
