/**
 * algebraic_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "../matrix/matrix_system.h"
#include "raw_sequence_book.h"

namespace NPATK {
    class AlgebraicContext;

    class AlgebraicMatrixSystem : public MatrixSystem {

    private:
        AlgebraicContext &algebraicContext;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<AlgebraicContext> context);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<class Context> context);

        /**
         * Generate substitution rules, for up to desired string length.
         * Will call write lock on mutex, so don't lock before.
         */
        void generate_aliases(size_t stringLength);



    };
}