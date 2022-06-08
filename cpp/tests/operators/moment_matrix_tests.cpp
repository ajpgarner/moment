/**
 * npa_matrix_Tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/moment_matrix.h"
#include "operators/context.h"

namespace NPATK::Tests {
    namespace {
        void compare_os_matrix(MomentMatrix &theMM, size_t dimension,
                              std::initializer_list<OperatorSequence> reference) {
            ASSERT_EQ(theMM.dimension(), dimension) << " level = " << theMM.level();
            ASSERT_EQ(theMM.dimensions().first, dimension) << " level = " << theMM.level();
            ASSERT_EQ(theMM.dimensions().second, dimension) << " level = " << theMM.level();
            size_t row = 0;
            size_t col = 0;
            for (const auto &ref_seq: reference) {
                ASSERT_LT(row, dimension) << " level = " << theMM.level() << ", row = " << row << ", col = " << col;
                ASSERT_LT(col, dimension) << " level = " << theMM.level() << ", row = " << row << ", col = " << col;

                const auto &actual_seq = theMM[row][col];
                EXPECT_EQ(actual_seq, ref_seq) << " level = " << theMM.level() << ", row = " << row << ", col = " << col;
                ++col;
                if (col >= dimension) {
                    col = 0;
                    ++row;
                }
            }
            EXPECT_EQ(col, 0) << " level = " << theMM.level();
            EXPECT_EQ(row, dimension) << " level = " << theMM.level();
        }


        struct unique_seq_brace_ref {
            OperatorSequence fwd;
            OperatorSequence rev;
            bool herm;
        };

        void compare_unique_sequences(MomentMatrix &theMM, std::initializer_list<unique_seq_brace_ref> reference) {
            ASSERT_EQ(theMM.UniqueSequences.size(), 2 + reference.size());

            // 0 is always zero
            auto iter = theMM.UniqueSequences.begin();
            ASSERT_NE(iter, theMM.UniqueSequences.end()) << " level = " << theMM.level();
            EXPECT_EQ(&(*iter), &theMM.UniqueSequences[0]) << " level = " << theMM.level();
            EXPECT_EQ(theMM.UniqueSequences[0].sequence(), OperatorSequence::Zero())
                << " level = " << theMM.level();
            EXPECT_EQ(theMM.UniqueSequences[0].sequence_conj(), OperatorSequence::Zero())
                << " level = " << theMM.level();
            EXPECT_TRUE(theMM.UniqueSequences[0].is_hermitian()) << " level = " << theMM.level();
            ++iter;

            // 1 is always ID
            ASSERT_NE(iter, theMM.UniqueSequences.end()) << " level = " << theMM.level();
            EXPECT_EQ(&(*iter), &theMM.UniqueSequences[1]) << " level = " << theMM.level();
            EXPECT_EQ(theMM.UniqueSequences[1].sequence(), OperatorSequence::Identity())
                << " level = " << theMM.level();
            EXPECT_EQ(theMM.UniqueSequences[1].sequence_conj(), OperatorSequence::Identity())
                << " level = " << theMM.level();
            EXPECT_TRUE(theMM.UniqueSequences[1].is_hermitian())  << " level = " << theMM.level();
            ++iter;

            size_t index = 2;
            for (const auto& ref_seq : reference) {
                ASSERT_NE(iter, theMM.UniqueSequences.end()) << " level = " << theMM.level() << ", index = " << index;
                EXPECT_EQ(&(*iter), &theMM.UniqueSequences[index]) << " level = " << theMM.level()
                    << ", index = " << index;
                EXPECT_EQ(iter->sequence(), ref_seq.fwd) << " level = " << theMM.level() << ", index = " << index;
                EXPECT_EQ(iter->sequence_conj(), ref_seq.rev) << " level = " << theMM.level() << ", index = " << index;
                EXPECT_EQ(iter->is_hermitian(), ref_seq.herm) << " level = " << theMM.level() << ", index = " << index;
                ++index;
                ++iter;
            }

            EXPECT_EQ(index, 2 + reference.size()) << " level = " << theMM.level();
            EXPECT_EQ(iter, theMM.UniqueSequences.end()) << " level = " << theMM.level();
        }

        void compare_symbol_matrix(MomentMatrix &theMM, size_t dimension,
                                   const std::vector<SymbolExpression>& reference) {
            ASSERT_EQ(theMM.SymbolMatrix.dimension(), dimension);
            ASSERT_EQ(theMM.SymbolMatrix.dimensions().first, dimension);
            ASSERT_EQ(theMM.SymbolMatrix.dimensions().second, dimension);

            size_t row = 0;
            size_t col = 0;
            for (const auto &ref_symbol: reference) {
                ASSERT_LT(row, dimension) << " level = " << theMM.level() << ", row = " << row << ", col = " << col;
                ASSERT_LT(col, dimension) << " level = " << theMM.level() << ", row = " << row << ", col = " << col;

                const auto &actual_symbol = theMM.SymbolMatrix[row][col];
                EXPECT_EQ(actual_symbol, ref_symbol)
                    << " level = " << theMM.level() << ", row = " << row << ", col = " << col;
                ++col;
                if (col >= dimension) {
                    col = 0;
                    ++row;
                }
            }
            EXPECT_EQ(col, 0) << " level = " << theMM.level();
            EXPECT_EQ(row, dimension) << " level = " << theMM.level();

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
        Context context(0, 0); // No parties, no symbols
        ASSERT_EQ(context.size(), 0);

        MomentMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.level(), 0);
        compare_os_matrix(matLevel0, 1, {OperatorSequence::Identity(&context)});
        compare_unique_sequences(matLevel0, {});
        compare_symbol_matrix(matLevel0, 1, {"1"});

        MomentMatrix matLevel1{context, 1};
        EXPECT_EQ(matLevel1.level(), 1);
        compare_os_matrix(matLevel1, 1, {OperatorSequence::Identity(&context)});
        compare_unique_sequences(matLevel1, {});
        compare_symbol_matrix(matLevel1, 1, {"1"});

        MomentMatrix matLevel5{context, 5};
        EXPECT_EQ(matLevel5.level(), 5);
        compare_os_matrix(matLevel5, 1, {OperatorSequence::Identity(&context)});
        compare_unique_sequences(matLevel5, {});
        compare_symbol_matrix(matLevel1, 1, {"1"});
    }

    TEST(MomentMatrix, OpSeq_OneElem) {
        Context context{1}; // One party, one symbol
        ASSERT_EQ(context.size(), 1);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);

        MomentMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.level(), 0);
        compare_os_matrix(matLevel0, 1, {OperatorSequence::Identity(&context)});


        MomentMatrix matLevel1{context, 1};
        EXPECT_EQ(matLevel1.level(), 1);
        compare_os_matrix(matLevel1, 2, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context)});


        MomentMatrix matLevel2{context, 2};
        EXPECT_EQ(matLevel2.level(), 2);
        compare_os_matrix(matLevel2, 3, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context)});
    }

    TEST(MomentMatrix, OpSeq_1Party2Opers) {
        Context context{2}; // One party, two symbols
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        MomentMatrix matLevel0{context, 0};
        compare_os_matrix(matLevel0, 1, {OperatorSequence::Identity(&context)});

        MomentMatrix matLevel1{context, 1};
        compare_os_matrix(matLevel1, 3, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[1]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1]}, &context)});

        MomentMatrix matLevel2{context, 2};
        compare_os_matrix(matLevel2, 7, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[0], alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[1], alice[1] , alice[1]}, &context),

                                         OperatorSequence({alice[1], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[0], alice[1]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[1], alice[0]}, &context),
                                         OperatorSequence({alice[1], alice[1], alice[1] , alice[1]}, &context)}

                 );
    };

    TEST(MomentMatrix, OpSeq_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        MomentMatrix matLevel0{context, 0};
        compare_os_matrix(matLevel0, 1, {OperatorSequence::Identity(&context)});

        MomentMatrix matLevel1{context, 1};
        compare_os_matrix(matLevel1, 3, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0]}, &context)});


        MomentMatrix matLevel2{context, 2};
        compare_os_matrix(matLevel2, 6, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0]}, &context),

                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0]}, &context),

                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0], bob[0]}, &context),

                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0], bob[0]}, &context),

                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0], bob[0]}, &context),

                                         OperatorSequence({bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0], bob[0], bob[0]}, &context),
                                         OperatorSequence({bob[0], bob[0], bob[0], bob[0]}, &context)});
    }

    TEST(MomentMatrix, OpSeq_2Party1OpersIdem) {
        Context context(2, 1, Operator::Flags::Idempotent); // Two party, one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        MomentMatrix matLevel0{context, 0};
        compare_os_matrix(matLevel0, 1, {OperatorSequence::Identity(&context)});

        MomentMatrix matLevel1{context, 1};
        compare_os_matrix(matLevel1, 3, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0]}, &context)});


        MomentMatrix matLevel2{context, 2};
        compare_os_matrix(matLevel2, 4, {OperatorSequence::Identity(&context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),

                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),

                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),

                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context),
                                         OperatorSequence({alice[0], bob[0]}, &context)});
    }

    TEST(MomentMatrix, Unique_OneElem) {
        Context context{1}; // One party, one symbol
        ASSERT_EQ(context.size(), 1);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);

        MomentMatrix matLevel0{context, 0};
        compare_unique_sequences(matLevel0, {});

        MomentMatrix matLevel1{context, 1};
        compare_unique_sequences(matLevel1, {{OperatorSequence({alice[0]}, &context),
                                                     OperatorSequence({alice[0]}, &context), true},
                                             {OperatorSequence({alice[0], alice[0]}, &context),
                                                     OperatorSequence({alice[0], alice[0]}, &context), true}});

        MomentMatrix matLevel2{context, 2};
        compare_unique_sequences(matLevel2,
                                 {{OperatorSequence({alice[0]}, &context),
                                          OperatorSequence({alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context), true}});
    }

    TEST(MomentMatrix, Unique_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.size(), 1);
        ASSERT_EQ(bob.size(), 1);


        MomentMatrix matLevel0{context, 0};
        compare_unique_sequences(matLevel0, {});

        MomentMatrix matLevel1{context, 1};
        compare_unique_sequences(matLevel1,
                                 {{OperatorSequence({alice[0]}, &context),
                                          OperatorSequence({alice[0]}, &context), true},
                                  {OperatorSequence({bob[0]}, &context),
                                          OperatorSequence({bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], bob[0]}, &context), true},
                                  {OperatorSequence({bob[0], bob[0]}, &context),
                                          OperatorSequence({bob[0], bob[0]}, &context), true}});


        MomentMatrix matLevel2{context, 2};
        compare_unique_sequences(matLevel2,
                                 {{OperatorSequence({alice[0]}, &context),
                                          OperatorSequence({alice[0]}, &context), true},
                                  {OperatorSequence({bob[0]}, &context),
                                          OperatorSequence({bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], bob[0]}, &context), true},
                                  {OperatorSequence({bob[0], bob[0]}, &context),
                                          OperatorSequence({bob[0], bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], bob[0], bob[0]}, &context), true},
                                  {OperatorSequence({bob[0], bob[0], bob[0]}, &context),
                                          OperatorSequence({bob[0], bob[0], bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], alice[0], bob[0]}, &context),
                                          OperatorSequence({alice[0],alice[0],  alice[0], bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], alice[0], bob[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], alice[0], bob[0], bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0], bob[0], bob[0]}, &context),
                                          OperatorSequence({alice[0], bob[0], bob[0], bob[0]}, &context), true},
                                  {OperatorSequence({bob[0], bob[0], bob[0], bob[0]}, &context),
                                          OperatorSequence({bob[0], bob[0], bob[0], bob[0]}, &context), true}
         });
    }

    TEST(MomentMatrix, Unique_2Party1OpersIdem) {
        Context context(2, 1, Operator::Flags::Idempotent); // Two party, one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);


        MomentMatrix matLevel0{context, 0};
        compare_unique_sequences(matLevel0, {});

        MomentMatrix matLevel1{context, 1};
        compare_unique_sequences(matLevel1,
                                 {{OperatorSequence({alice[0]}, &context),
                                   OperatorSequence({alice[0]}, &context), true},
                                  {OperatorSequence({bob[0]}, &context),
                                   OperatorSequence({bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0]}, &context),
                                   OperatorSequence({alice[0], bob[0]}, &context), true}});

        MomentMatrix matLevel2{context, 2};
        compare_unique_sequences(matLevel2,
                                 {{OperatorSequence({alice[0]}, &context),
                                   OperatorSequence({alice[0]}, &context), true},
                                  {OperatorSequence({bob[0]}, &context),
                                   OperatorSequence({bob[0]}, &context), true},
                                  {OperatorSequence({alice[0], bob[0]}, &context),
                                   OperatorSequence({alice[0], bob[0]}, &context), true}});
    }

    TEST(MomentMatrix, Unique_1Party2Opers) {
        Context context{2}; // One party, two symbols
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        MomentMatrix matLevel0{context, 0};
        compare_unique_sequences(matLevel0, {});

        MomentMatrix matLevel1{context, 1};
        compare_unique_sequences(matLevel1, {{OperatorSequence({alice[0]}, &context),
                                                     OperatorSequence({alice[0]}, &context), true},
                                             {OperatorSequence({alice[1]}, &context),
                                                     OperatorSequence({alice[1]}, &context), true},
                                             {OperatorSequence({alice[0], alice[0]}, &context),
                                                     OperatorSequence({alice[0], alice[0]}, &context), true},
                                             {OperatorSequence({alice[0], alice[1]}, &context),
                                                     OperatorSequence({alice[1], alice[0]}, &context), false},
                                             {OperatorSequence({alice[1], alice[1]}, &context),
                                                     OperatorSequence({alice[1], alice[1]}, &context), true}});

        MomentMatrix matLevel2{context, 2};
        compare_unique_sequences(matLevel2, {
                {OperatorSequence({alice[0]}, &context),
                        OperatorSequence({alice[0]}, &context), true},
                {OperatorSequence({alice[1]}, &context),
                        OperatorSequence({alice[1]}, &context), true},

                {OperatorSequence({alice[0], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[0]}, &context), false},
                {OperatorSequence({alice[1], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[1]}, &context), true},

                {OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[0], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[0], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[1], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[1], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[1], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[1], alice[0]}, &context), false},
                {OperatorSequence({alice[1], alice[0], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[0], alice[1]}, &context), true},
                {OperatorSequence({alice[1], alice[1], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[1], alice[1]}, &context), true},

                {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[0], alice[0], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[1], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[0]} , &context), false},
                {OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , &context), true},
                {OperatorSequence({alice[1], alice[0], alice[1], alice[0]} , &context),
                        OperatorSequence({alice[0], alice[1], alice[0], alice[1]} , &context), false},
                {OperatorSequence({alice[1], alice[0], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[1]} , &context), false},
                {OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , &context), true}
        });
    };

    TEST(MomentMatrix, Where_1Party2Opers) {
        Context context{2}; // One parties with two operators.
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        MomentMatrix matLevel2{context, 2};

        auto ptr_a0a0a0a0 = matLevel2.UniqueSequences.where(OperatorSequence{alice[0], alice[0], alice[0], alice[0]});
        ASSERT_NE(ptr_a0a0a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a0a0->sequence(), (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));

        auto ptr_a0a0a1a1 = matLevel2.UniqueSequences.where(OperatorSequence{alice[0], alice[0], alice[1], alice[1]});
        auto ptr_a1a1a0a0 = matLevel2.UniqueSequences.where(OperatorSequence{alice[1], alice[1], alice[0], alice[0]});
        ASSERT_NE(ptr_a0a0a1a1, nullptr);
        ASSERT_NE(ptr_a1a1a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a1a1, ptr_a1a1a0a0);

        EXPECT_EQ(ptr_a0a0a1a1->sequence(), (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence(), (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(ptr_a0a0a1a1->sequence_conj(), (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence_conj(), (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));

        auto ptr_a0a0a0a0a0 = matLevel2.UniqueSequences.where(OperatorSequence{alice[0], alice[0], alice[0], alice[0], alice[0]});
        EXPECT_EQ(ptr_a0a0a0a0a0, nullptr);
    }

    TEST(MomentMatrix, Symbol_OneElem) {
        Context context{1}; // One party, one symbol

        MomentMatrix matLevel0{context, 0}; // id
        compare_symbol_matrix(matLevel0, 1, {"1"});

        MomentMatrix matLevel1{context, 1}; // id, a, a^2
        compare_symbol_matrix(matLevel1, 2, {"1", "2",
                                             "2", "3"});

        MomentMatrix matLevel2{context, 2}; // id, a, a^2, a^3, a^4
        compare_symbol_matrix(matLevel2, 3, {"1", "2", "3",
                                             "2", "3", "4",
                                             "3", "4", "5"});
    }

    TEST(MomentMatrix, Symbol_1Party2Opers) {
        Context context{2}; // One party, two symbols

        MomentMatrix matLevel0{context, 0};
        compare_symbol_matrix(matLevel0, 1, {"1"});

        MomentMatrix matLevel1{context, 1}; // x, 0, 1???
        compare_symbol_matrix(matLevel1, 3, {"1",  "2", "3",
                                             "2",  "4", "5",
                                             "3", "5*", "6"});

        MomentMatrix matLevel2{context, 2};
        compare_symbol_matrix(matLevel2, 7, // Remember symbol order is from hash function...
                              {"1",  "2",   "3",   "4",   "5",  "5*", "6",  // x, 0,  1,  00,  01,  10,  11
                               "2",  "4",   "5",   "7",   "8",  "9",  "10", // 0, 00, 01, 000, 001, 010, 011
                               "3",  "5*",  "6",   "8*",  "11", "10*", "12", // 1, 10, 11, 100, 101, 110, 111
                               "4",  "7",   "8",   "13",  "14", "15", "16", // 001, 000, 001, 0000, 0001, 0010, 0011
                               "5*", "8*",  "11",  "14*", "19", "20", "21", // 10, 100, 101, 1000, 1001, 1010, 1011
                               "5",  "9",   "10",  "15*", "20*","17", "18", // 01, 010, 011, 0100, 0101, 0110, 0111
                               "6",  "10*", "12",  "16*", "21*","18*","22" // 11, 110, 111, 1100, 1101, 1110, 1111
        });
    };

    TEST(MomentMatrix, Symbol_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator

        MomentMatrix matLevel0{context, 0};
        compare_symbol_matrix(matLevel0, 1, {"1"});

        MomentMatrix matLevel1{context, 1};
        compare_symbol_matrix(matLevel1, 3, {"1", "2", "3", // 1, a, b
                                             "2", "4", "5", // a, aa, ab
                                             "3", "5", "6"}); // b, ab, bb

        MomentMatrix matLevel2{context, 2};
        compare_symbol_matrix(matLevel2, 6, {"1", "2", "3",  "4", "5",  "6",   //  1,  a,    b,   aa,   ab, bb
                                             "2", "4", "5",  "7", "8",  "9",   //  a,  aa,  ab,  aaa,  aab, abb
                                             "3", "5", "6",  "8", "9",  "10",  //  b,  ab,  bb,  aab,  abb, bbb
                                             "4", "7", "8",  "11", "12","13",  // aa, aaa, aab, aaaa, aaab, aabb
                                             "5", "8", "9",  "12", "13", "14", // ab, aab, abb, aaab, aabb, abbb
                                             "6", "9", "10", "13", "14", "15", // bb, abb, bbb, aabb, abbb, bbbb
        });
    }

    TEST(MomentMatrix, Symbol_2Party1OpersIdem) {
        Context context(2, 1, Operator::Flags::Idempotent); // Two party, one operator
        MomentMatrix matLevel0{context, 0};
        compare_symbol_matrix(matLevel0, 1, {"1"});

        MomentMatrix matLevel1{context, 1};
        compare_symbol_matrix(matLevel1, 3, {"1", "2", "3",   // 1, a, b
                                             "2", "2", "4",   // a, aa, ab
                                             "3", "4", "3"}); // b, ab, b

        MomentMatrix matLevel2{context, 2}; // order of unique symbols: 1, a, b, ab
        compare_symbol_matrix(matLevel2, 4, {"1", "2", "3", "4",  // 1, a, b, ab
                                             "2", "2", "4", "4",  // a, a, ab, ab
                                             "3", "4", "3", "4",  // b, ab, b, ab
                                             "4", "4", "4", "4"});// ab, ab, ab, ab
    }

    TEST(MomentMatrix, ToSymbol_1Party2Opers) {
        Context context{2}; // One party, two symbols
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& a = context.Parties[0];

        MomentMatrix matLevel0{context, 0}; // 0 1
        EXPECT_EQ(matLevel0.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel0.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));

        MomentMatrix matLevel1{context, 1}; // 0 1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(matLevel1.UniqueSequences.size(), 7);
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[0]})), SymbolExpression(2));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[1]})), SymbolExpression(3));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[0], a[0]})), SymbolExpression(4));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[0], a[1]})), SymbolExpression(5));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[1], a[0]})), SymbolExpression(5, true));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{a[1], a[1]})), SymbolExpression(6));

        MomentMatrix matLevel2{context, 2};
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0]}, &context)), SymbolExpression(2));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1]}, &context)), SymbolExpression(3));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0]}, &context)), 
                  SymbolExpression(4));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1]}, &context)),
                  SymbolExpression(5));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0]}, &context)), 
                  SymbolExpression(5, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1]}, &context)), 
                  SymbolExpression(6));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[0]}, &context)), 
                  SymbolExpression(7));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[1]}, &context)), 
                  SymbolExpression(8));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[0]}, &context)), 
                  SymbolExpression(8, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[0]}, &context)), 
                  SymbolExpression(9));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[1]}, &context)), 
                  SymbolExpression(10));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[0]}, &context)),
                  SymbolExpression(10, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[1]}, &context)), 
                  SymbolExpression(11));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[1]}, &context)), 
                  SymbolExpression(12));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[0], a[0]}, &context)), 
                  SymbolExpression(13));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[0], a[1]}, &context)),
                  SymbolExpression(14));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[0], a[0]}, &context)),
                  SymbolExpression(14, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[1], a[0]}, &context)), 
                  SymbolExpression(15));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[0], a[0]}, &context)), 
                  SymbolExpression(15, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[0], a[1], a[1]}, &context)), 
                  SymbolExpression(16));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[0], a[0]}, &context)), 
                  SymbolExpression(16, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[1], a[0]}, &context)), 
                  SymbolExpression(17));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[1], a[1]} , &context)), 
                  SymbolExpression(18));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[1], a[0]} , &context)), 
                  SymbolExpression(18, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[0], a[1]} , &context)), 
                  SymbolExpression(19));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[1], a[0]} , &context)), 
                  SymbolExpression(20));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[0], a[1], a[0], a[1]} , &context)), 
                  SymbolExpression(20, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[0], a[1], a[1]} , &context)), 
                  SymbolExpression(21));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[0], a[1]} , &context)), 
                  SymbolExpression(21, true));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence({a[1], a[1], a[1], a[1]} , &context)), 
                  SymbolExpression(22));
    };

    TEST(MomentMatrix, ToSymbol_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        MomentMatrix matLevel0{context, 0}; //0 1
        EXPECT_EQ(matLevel0.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel0.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));

        MomentMatrix matLevel1{context, 1}; // 0 1 a b aa ab bb
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{alice[0]})), SymbolExpression(2));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{bob[0]})), SymbolExpression(3));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0]})), SymbolExpression(4));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{alice[0], bob[0]})), SymbolExpression(5));
        EXPECT_EQ(matLevel1.UniqueSequences.to_symbol((OperatorSequence{bob[0], bob[0]})), SymbolExpression(6));

        MomentMatrix matLevel2{context, 2}; // 0 1 aaaa aaab aabb abbb bbbb
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence::Zero(&context)), SymbolExpression(0));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol(OperatorSequence::Identity(&context)), SymbolExpression(1));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0]})), SymbolExpression(2));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{bob[0]})), SymbolExpression(3));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0]})), SymbolExpression(4));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], bob[0]})), SymbolExpression(5));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{bob[0], bob[0]})), SymbolExpression(6));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0], alice[0]})),
                  SymbolExpression(7));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0], bob[0]})),
                  SymbolExpression(8));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], bob[0], bob[0]})),
                  SymbolExpression(9));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{bob[0], bob[0], bob[0]})),
                  SymbolExpression(10));

        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0], alice[0], alice[0]})),
                  SymbolExpression(11));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0],alice[0],alice[0],bob[0]})),
                  SymbolExpression(12));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], alice[0], bob[0], bob[0]})),
                  SymbolExpression(13));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{alice[0], bob[0], bob[0], bob[0]})),
                  SymbolExpression(14));
        EXPECT_EQ(matLevel2.UniqueSequences.to_symbol((OperatorSequence{bob[0], bob[0], bob[0], bob[0]})),
                  SymbolExpression(15));
    }
}