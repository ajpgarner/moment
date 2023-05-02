/**
 * lu_map_core_processor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 */
#pragma once
#include "map_core.h"

namespace Moment::Derived {
    /**
     * Lower/Upper-decomposition based map core processor.
     *
     * The general idea of this class is to identify a rank-deficiency in the mapping matrix, so that we can replace one
     * symbol table with another with fewer symbols.
     *
     * This can be done by taking an LU decomposition of the mapping matrix's transpose (X').
     * In particular, let [L, U] = lu(X') be the lower and upper triangular decomposition (up to some permutation).
     * Then, U is a matrix whose rows defines the new symbols in terms of the old ones; and L is a matrix whose rows
     * tells us which elements of y we should replace the original (pre-transformation) symbols in x with.
     */
    class LUMapCoreProcessor : public MapCoreProcessor {
    public:
        std::unique_ptr<SolvedMapCore> operator()(const DenseMapCore& core) const final;

        std::unique_ptr<SolvedMapCore> operator()(const SparseMapCore& core) const final;
    };
}