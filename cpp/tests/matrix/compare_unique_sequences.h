/**
 * compare_unique_sequences.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "gtest/gtest.h"

#include "dictionary/operator_sequence.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/monomial_matrix.h"

#include "symbolic/symbol_table.h"

namespace Moment::Tests {
    struct unique_seq_brace_ref {
        OperatorSequence fwd;
        OperatorSequence rev;
        bool herm;
    };

    inline void compare_unique_sequences(const SymbolicMatrix &theMM, std::initializer_list<unique_seq_brace_ref> reference) {
        ASSERT_EQ(theMM.symbols.size(), 2 + reference.size());

        const auto* mmPtr = MomentMatrix::as_monomial_moment_matrix_ptr(theMM);
        ASSERT_NE(mmPtr, nullptr) << "Not a moment matrix!";


        // 0 is always zero
        auto iter = theMM.symbols.begin();
        ASSERT_NE(iter, theMM.symbols.end()) << " Level = " << mmPtr->Level();
        EXPECT_EQ(&(*iter), &theMM.symbols[0]) << " Level = " << mmPtr->Level();
        EXPECT_EQ(theMM.symbols[0].sequence(), OperatorSequence::Zero(theMM.context))
                << " Level = " << mmPtr->Level();
        EXPECT_EQ(theMM.symbols[0].sequence_conj(), OperatorSequence::Zero(theMM.context))
                << " Level = " << mmPtr->Level();
        EXPECT_TRUE(theMM.symbols[0].is_hermitian()) << " Level = " << mmPtr->Level();
        ++iter;

        // 1 is always ID
        ASSERT_NE(iter, theMM.symbols.end()) << " Level = " << mmPtr->Level();
        EXPECT_EQ(&(*iter), &theMM.symbols[1]) << " Level = " << mmPtr->Level();
        EXPECT_EQ(theMM.symbols[1].sequence(), OperatorSequence::Identity(theMM.context))
                << " Level = " << mmPtr->Level();
        EXPECT_EQ(theMM.symbols[1].sequence_conj(), OperatorSequence::Identity(theMM.context))
                << " Level = " << mmPtr->Level();
        EXPECT_TRUE(theMM.symbols[1].is_hermitian())  << " Level = " << mmPtr->Level();
        ++iter;

        size_t index = 2;
        for (const auto& ref_seq : reference) {
            ASSERT_NE(iter, theMM.symbols.end()) << " Level = " << mmPtr->Level() << ", index = " << index;
            EXPECT_EQ(&(*iter), &theMM.symbols[index]) << " Level = " << mmPtr->Level()
                                                       << ", index = " << index;
            EXPECT_EQ(iter->sequence(), ref_seq.fwd) << " Level = " << mmPtr->Level() << ", index = " << index;
            EXPECT_EQ(iter->sequence_conj(), ref_seq.rev) << " Level = " << mmPtr->Level() << ", index = " << index;
            EXPECT_EQ(iter->is_hermitian(), ref_seq.herm) << " Level = " << mmPtr->Level() << ", index = " << index;
            ++index;
            ++iter;
        }

        EXPECT_EQ(index, 2 + reference.size()) << " Level = " << mmPtr->Level();
        EXPECT_EQ(iter, theMM.symbols.end()) << " Level = " << mmPtr->Level();
    }
}
