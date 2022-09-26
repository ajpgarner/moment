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
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

#include "localizing_matrix_index.h"

namespace NPATK {

    class Context;
    class SymbolTable;
    class ExplicitSymbolIndex;
    class ImplicitSymbols;
    class CollinsGisin;
    class OperatorMatrix;
    class LocalizingMatrix;
    class MomentMatrix;

    namespace errors {
        class missing_component : public std::runtime_error {
        public:
            explicit missing_component(const std::string& what) : std::runtime_error{what} { }
        };
    }

    class MatrixSystem {
    private:
        std::unique_ptr<class Context> context;

        std::unique_ptr<SymbolTable> symbol_table;

        std::vector<std::unique_ptr<OperatorMatrix>> matrices;

        std::vector<ptrdiff_t> momentMatrixIndices;

        std::map<LocalizingMatrixIndex, ptrdiff_t> localizingMatrixIndices;

        /** Map of measurement outcome symbols */
        std::unique_ptr<ExplicitSymbolIndex> explicitSymbols;

        /** Map of implied probabilities */
        std::unique_ptr<ImplicitSymbols> implicitSymbols;

        /** Map of outcome symbols, by Collins Gisin index */
        std::unique_ptr<class CollinsGisin> collinsGisin;

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
         * Check if a MomentMatrix has been generated for a particular hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return True, if MomentMatrix exists for particular Level.
         */
        [[nodiscard]] bool hasMomentMatrix(size_t level) const noexcept;

        /**
         * Returns the MomentMatrix for a particular hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return The MomentMatrix for this particular Level.
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const class MomentMatrix& MomentMatrix(size_t level) const;

        /**
         * Constructs a moment matrix for a particular Level, or returns pre-existing one.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param level The hierarchy depth.
         * @return The MomentMatrix for this particular Level.
         */
        class NPATK::MomentMatrix& CreateMomentMatrix(size_t level);

        /**
         * Check if a localizing matrix has been generated for a particular sequence and hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @return The numerical index within this matrix system, or -1 if not found.
         */
        [[nodiscard]] ptrdiff_t localizingMatrixIndex(const LocalizingMatrixIndex& lmi) const noexcept;

        /**
         * Gets the localizing matrix for a particular sequence and hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @return The MomentMatrix for this particular Level.
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const class LocalizingMatrix& LocalizingMatrix(const LocalizingMatrixIndex& lmi) const;

        /**
         * Constructs a localizing matrix for a particular Level on a particular word, or returns a pre-existing one.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param level The hierarchy depth.
         * @param word The word
         * @return The LocalizingMatrix
         */
        class LocalizingMatrix& CreateLocalizingMatrix(LocalizingMatrixIndex lmi);

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
         * Returns an indexing in the Collins-Gisin ordering.
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const class CollinsGisin& CollinsGisin() const;

        /**
         * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
         * the context (including joint measurements).
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const ExplicitSymbolIndex& ExplicitSymbolTable() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements).
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const ImplicitSymbols& ImplicitSymbolTable() const;
    };
}
