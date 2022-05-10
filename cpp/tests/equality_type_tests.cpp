/**
 * equality_type_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "equality_type.h"

namespace NPATK::Tests {

    TEST(EqualityType, DistinctValues) {
        EXPECT_NE(EqualityType::none, EqualityType::equal);
        EXPECT_NE(EqualityType::none, EqualityType::negated);
        EXPECT_NE(EqualityType::none, EqualityType::conjugated);
        EXPECT_NE(EqualityType::none, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::equal, EqualityType::negated);
        EXPECT_NE(EqualityType::equal, EqualityType::conjugated);
        EXPECT_NE(EqualityType::equal, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::negated, EqualityType::conjugated);
        EXPECT_NE(EqualityType::negated, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::conjugated, EqualityType::neg_conj);
    }

    TEST(EqualityType, Or) {
        EXPECT_EQ(EqualityType::none | EqualityType::equal, EqualityType::equal);
        EXPECT_EQ(EqualityType::none | EqualityType::negated, EqualityType::negated);
        EXPECT_EQ(EqualityType::none | EqualityType::conjugated, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::none | EqualityType::neg_conj, EqualityType::neg_conj);

        EXPECT_EQ(EqualityType::equal | EqualityType::none, EqualityType::equal);
        EXPECT_EQ(EqualityType::negated | EqualityType::none, EqualityType::negated);
        EXPECT_EQ(EqualityType::conjugated | EqualityType::none, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::neg_conj | EqualityType::none, EqualityType::neg_conj);
    }

    TEST(EqualityType, And) {
        // Idempotent:
        EXPECT_EQ(EqualityType::none & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::equal, EqualityType::equal);
        EXPECT_EQ(EqualityType::negated & EqualityType::negated, EqualityType::negated);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::conjugated, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::neg_conj, EqualityType::neg_conj);

        // Mutual exclusion:
        EXPECT_EQ(EqualityType::none & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::conjugated, EqualityType::none);
    }

    TEST(EqualityType, CreateFromPair) {
        EXPECT_EQ(equality_type(SymbolPair{Symbol{1}, Symbol{2}}), EqualityType::equal);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{1}, Symbol{-2}}), EqualityType::negated);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{1}, Symbol{2, true}}), EqualityType::conjugated);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{1}, Symbol{-2, true}}), EqualityType::neg_conj);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{-1}, Symbol{2}}), EqualityType::negated);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{1, true}, Symbol{2}}), EqualityType::conjugated);
        EXPECT_EQ(equality_type(SymbolPair{Symbol{-1, true}, Symbol{2}}), EqualityType::neg_conj);
    }

    TEST(EqualityType, Negate) {
        EXPECT_EQ(negate(EqualityType::equal), EqualityType::negated);
        EXPECT_EQ(negate(EqualityType::negated), EqualityType::equal);
        EXPECT_EQ(negate(EqualityType::conjugated), EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::neg_conj), EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::negated),
                  EqualityType::equal | EqualityType::negated);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::conjugated),
                  EqualityType::negated | EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::neg_conj),
                  EqualityType::negated | EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::negated | EqualityType::conjugated),
                  EqualityType::equal | EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::negated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::conjugated | EqualityType::neg_conj);

        EXPECT_EQ(negate(EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj);
    }

    TEST(EqualityType, Conjugate) {
        EXPECT_EQ(conjugate(EqualityType::equal), EqualityType::conjugated);
        EXPECT_EQ(conjugate(EqualityType::negated), EqualityType::neg_conj);
        EXPECT_EQ(conjugate(EqualityType::conjugated), EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::neg_conj), EqualityType::negated);

        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::negated),
                  EqualityType::conjugated | EqualityType::neg_conj);
        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::conjugated),
                  EqualityType::conjugated | EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::neg_conj),
                  EqualityType::conjugated | EqualityType::negated);
        EXPECT_EQ(conjugate(EqualityType::negated | EqualityType::conjugated),
                  EqualityType::neg_conj | EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::negated | EqualityType::neg_conj),
                  EqualityType::neg_conj | EqualityType::negated);
        EXPECT_EQ(conjugate(EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated);

        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj);
    }

    TEST(EqualityType, Compose) {
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::equal), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::negated), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::conjugated), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::neg_conj), EqualityType::neg_conj);

        EXPECT_EQ(compose(EqualityType::negated, EqualityType::equal), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::negated), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::conjugated), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::neg_conj), EqualityType::conjugated);

        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::equal), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::negated), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::conjugated), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::neg_conj), EqualityType::negated);

        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::equal), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::negated), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::conjugated), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::neg_conj), EqualityType::equal);
    }

}