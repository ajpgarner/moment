/**
 * lu_map_core_processor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * The general idea of this class is to identify a rank-deficiency in the mapping matrix, so that we can replace one
 * symbol table with another with fewer symbols.
 *
 * This can be done by taking an LU decomposition of the mapping matrix's transpose (X').
 * In particular, let [L, U] = lu(X') be the lower and upper triangular decomposition. Then, U is a matrix whose rows
 * defines the new symbols in terms of the old ones; and L is a matrix whose rows tells us which elements of
 * y we should replace the original (pre-tansformation) symbols in x with.
 */
#pragma once
#include "map_core.h"

namespace Moment::Symmetrized {
    class LUMapCoreProcessor : public MapCoreProcessor {
    public:
        SolvedMapCore operator()(const MapCore& core) final;
    };
}