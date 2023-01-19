/**
 * matrix_type.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

namespace Moment {
    /** Matrix type */
    enum class MatrixType {
        Unknown = 0,
        /** Real-valued */
        Real = 1,
        /** Complex-valued */
        Complex = 2,
        /** Real-valued, matrix is symmetric */
        Symmetric = 3,
        /** Complex-valued, matrix is hermitian */
        Hermitian = 4
    };
}