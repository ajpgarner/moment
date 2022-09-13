/**
 * matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * FOR THREAD SAFETY: Functions accessing a matrix system should call for the read lock before accessing anything.
 */
#pragma once

#include <cassert>

#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>

namespace NPATK {

    class Context;
    class SymbolTable;
    class CollinsGisinIndex;
    class ImplicitSymbols;
    class OperatorMatrix;
    class MomentMatrix;


    class MatrixSystem {
    private:
        std::unique_ptr<class Context> context;

        std::unique_ptr<SymbolTable> symbol_table;

        std::vector<std::unique_ptr<OperatorMatrix>> matrices;

        std::vector<ptrdiff_t> momentMatrixIndices;

        /** Map of measurement outcome symbols */
        std::unique_ptr<CollinsGisinIndex> cgForm;

        /** Map of implied probabilities */
        std::unique_ptr<ImplicitSymbols> implicitSymbols;

        /** Read-write mutex for matrices */
        mutable std::shared_mutex rwMutex;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit MatrixSystem(std::unique_ptr<class Context> context);

        /**
         * Frees a system of matrices.
         */
        ~MatrixSystem() noexcept;

        /**
         * Gets a read lock on the matrix system.
         */
        auto getReadLock() const {
            return std::shared_lock{rwMutex};
        }

        /**
         * Access matrix by subscript corresponding to order of creation.
         * For thread safety, call for a read lock first.
         */
        const OperatorMatrix& operator[](size_t index) const {
            assert(index < this->matrices.size());
            return *this->matrices[index];
        }

        /**
         * Calculates the longest real sequences that can exist within this system (i.e. the highest number of
         *  parties, all of whose joint measurement outcomes correspond to symbols within.)
         */
        [[nodiscard]] size_t MaxRealSequenceLength() const noexcept;

        /**
         * Check if a MomentMatrix has been generated for a particular hierarchy level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return True, if MomentMatrix exists for particular level.
         */
        [[nodiscard]] bool hasMomentMatrix(size_t level) const noexcept;

        /**
         * Returns the MomentMatrix for a particular hierarchy level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return The MomentMatrix for this particular level.
         */
        [[nodiscard]] const class MomentMatrix& MomentMatrix(size_t level) const;

        /**
         * Constructs a moment matrix for a particular level, or returns pre-existing one.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param level The hierarchy depth.
         * @return The MomentMatrix for this particular level.
         */
        class MomentMatrix& CreateMomentMatrix(size_t level);

        /**
         * Returns the symbol table.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const SymbolTable& Symbols() const noexcept {
            return *this->symbol_table;
        }

        /**
         * Returns the context.
         * Can be called thread-safely, since Context is immutable once MatrixSystem is constructed.
         */
        [[nodiscard]] const class Context& Context() const noexcept {
            return *this->context;
        }

        /**
         * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
         * the context (including joint measurements).
         */
        [[nodiscard]] const CollinsGisinIndex& CollinsGisin() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from CollinsGisin(), but also implied "final"
         * outcomes of measurements (including joint measurements).
         */
        [[nodiscard]] const ImplicitSymbols& ImplicitSymbolTable() const;
    };
}
