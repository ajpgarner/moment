/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"
#include "../operator_sequence.h"

#include "operator_matrix.h"
#include "symbol_table.h"

#include "symbolic/symbol_expression.h"
#include "utilities/square_matrix.h"

#include <cassert>
#include <atomic>
#include <map>
#include <memory>
#include <span>

namespace NPATK {

    class CollinsGisinForm;
    class ImplicitSymbols;

    class MomentMatrix : public OperatorMatrix {
    public:

        /** The level of moment matrix defined */
        const size_t hierarchy_level;

        /**
         * The maximum length operator sequence found in this moment matrix that corresponds to a probability.
         * Effectively: min(number of parties, 2 * hierarchy_level)
         */
        const size_t max_probability_length;

    private:
        /** Map of measurement outcome symbols */
        std::unique_ptr<CollinsGisinForm> cgForm;

        /** Map of implied probabilities */
        std::unique_ptr<ImplicitSymbols> implicitSymbols;

        /** For thread-safe construction of cgForm */
        mutable std::atomic_flag cgFormExists;
        mutable std::atomic_flag cgFormConstructFlag;

        /** For thread-safe construction of implicit symbols */
        mutable std::atomic_flag impSymExists;
        mutable std::atomic_flag impSymConstructFlag;

    public:
        MomentMatrix(const Context& context, SymbolTable& symbols, size_t level);

        MomentMatrix(const MomentMatrix&) = delete;

        MomentMatrix(MomentMatrix&& src) noexcept;

        /** Destructor */
        ~MomentMatrix();

        [[nodiscard]] constexpr size_t level() const noexcept { return this->hierarchy_level; }

        [[nodiscard]] const CollinsGisinForm& CollinsGisin() const;

        [[nodiscard]] const ImplicitSymbols& ImplicitSymbolTable() const;

    private:
        std::map<size_t, UniqueSequence> identifyUniqueSequences(const std::vector<size_t> &hashes);

        std::unique_ptr<SquareMatrix<SymbolExpression>> buildSymbolMatrix(const std::vector<size_t> &hashes);
    };


}