/**
 * matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * FOR THREAD SAFETY: Functions accessing a matrix system should call for the read lock before accessing anything.
 */
#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

#include "matrix/localizing_matrix_index.h"

namespace NPATK {

    class Context;
    class SymbolTable;

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
        /** The operator context */
        std::unique_ptr<class Context> context;

        /** Map from symbols to operator sequences, and real/imaginary indices */
        std::unique_ptr<SymbolTable> symbol_table;

        /** List of matrices in the system */
        std::vector<std::unique_ptr<OperatorMatrix>> matrices;

        /** The index (in this->matrices) of generated moment matrices */
        std::vector<ptrdiff_t> momentMatrixIndices;

        /** The index (in this->matrices) of generated localizing matrices */
        std::map<LocalizingMatrixIndex, ptrdiff_t> localizingMatrixIndices;

    private:
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
        virtual ~MatrixSystem() noexcept;

        /**
         * Returns the symbol table.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const SymbolTable& Symbols() const noexcept {
            return *this->symbol_table;
        }

        /**
         * Returns the symbol table.
         * For thread safety, call for write lock before making changes.
         */
        [[nodiscard]] SymbolTable& Symbols() noexcept {
            return *this->symbol_table;
        }

        /**
         * Returns the context.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const class Context& Context() const noexcept {
            return *this->context;
        }

        /**
         * Returns the context.
         * For thread safety, call for write lock before making changes.
         */
        [[nodiscard]] class Context& Context() noexcept {
            return *this->context;
        }

        /**
         * Returns the MomentMatrix for a particular hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return The MomentMatrix for this particular Level.
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const class MomentMatrix& MomentMatrix(size_t level) const;

        /**
         * Gets the localizing matrix for a particular sequence and hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @return The MomentMatrix for this particular Level.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalizingMatrix& LocalizingMatrix(const LocalizingMatrixIndex& lmi) const;

        /**
         * Access matrix by subscript corresponding to order of creation.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const OperatorMatrix& operator[](size_t index) const {
            if (index >= this->matrices.size()) {
                throw errors::missing_component("Matrix index out of range.");
            }
            if (!this->matrices[index]) {
                throw errors::missing_component("Matrix at supplied index was missing.");
            }
            return *this->matrices[index];
        }

        /**
         * Counts matrices in system.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] size_t size() const noexcept {
            return this->matrices.size();
        }

        /**
         * Tests if there are any matrices in the system.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] bool empty() const noexcept {
            return this->matrices.empty();
        }

        /**
         * Returns the highest moment matrix yet generated.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] ptrdiff_t highest_moment_matrix() const noexcept;

        /**
          * Constructs a moment matrix for a particular Level, or returns pre-existing one.
          * Will lock until all read locks have expired - so do NOT first call for a read lock...!
          * @param level The hierarchy depth.
          * @return Pair: Matrix index and created MomentMatrix object reference.
          */
        std::pair<size_t, class MomentMatrix&> create_moment_matrix(size_t level);

        /**
         * Constructs a localizing matrix for a particular Level on a particular word, or returns a pre-existing one.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param level The hierarchy depth.
         * @param word The word
         * @return Pair: Matrix index and created LocalizingMatrix object reference.
         */
        std::pair<size_t, class LocalizingMatrix&> create_localizing_matrix(const LocalizingMatrixIndex& lmi);

        /**
         * Check if a MomentMatrix has been generated for a particular hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param level The hierarchy depth.
         * @return The numerical index within this matrix system, or -1 if not found.
         */
        [[nodiscard]] ptrdiff_t find_moment_matrix(size_t level) const noexcept;

        /**
         * Check if a localizing matrix has been generated for a particular sequence and hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @return The numerical index within this matrix system, or -1 if not found.
         */
        [[nodiscard]] ptrdiff_t find_localizing_matrix(const LocalizingMatrixIndex& lmi) const noexcept;

        /**
         * Gets a read lock for the matrix system.
         */
        [[nodiscard]] auto get_read_lock() const {
            return std::shared_lock{this->rwMutex};
        }

        /**
         * Gets a write lock for the matrix system.
         */
        [[nodiscard]] auto get_write_lock() {
            return std::unique_lock{this->rwMutex};
        }

    protected:
        virtual void beforeNewMomentMatrixCreated(size_t level) { }

        virtual void onNewMomentMatrixCreated(size_t level, const class MomentMatrix& mm) { }

        virtual void beforeNewLocalizingMatrixCreated(const LocalizingMatrixIndex& lmi) { }

        virtual void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex& lmi,
                                                  const class LocalizingMatrix& lm) { }

    };
}
