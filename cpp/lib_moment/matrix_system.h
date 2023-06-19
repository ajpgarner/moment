/**
 * matrix_system.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
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

#include "matrix/operator_matrix/localizing_matrix_index.h"

#include "utilities/multithreading.h"

namespace Moment {

    class Context;
    class PolynomialFactory;
    class OperatorSequenceGenerator;
    class SymbolTable;

    class WordList;
    class MomentSubstitutionRulebook;

    class Matrix;

    namespace errors {
        /**
         * Error issued when a component from the matrix system is requested, but does not exist.
         */
        class missing_component : public std::runtime_error {
        public:
            explicit missing_component(const std::string& what) : std::runtime_error{what} { }
        };
    }

    /**
     * Base class for systems of operators, and their associated moment/localizing matrices.
     *
     * FOR THREAD SAFETY: Functions accessing a matrix system should call for the read lock before accessing anything.
     */
    class MatrixSystem {
    private:
        /** The operator context */
        std::unique_ptr<class Context> context;

        /** Map from symbols to operator sequences, and real/imaginary indices */
        std::unique_ptr<SymbolTable> symbol_table;

        /** Factory object for constructing polynomials */
        std::unique_ptr<PolynomialFactory> poly_factory;

        /** List of matrices in the system. */
        std::vector<std::unique_ptr<Matrix>> matrices;

        /** List of moment substitution rulebooks in the system. */
        std::vector<std::unique_ptr<MomentSubstitutionRulebook>> rulebooks;

        /** The index (in this->matrices) of generated moment matrices. */
        std::vector<ptrdiff_t> momentMatrixIndices;

        /** The index (in this->matrices) of generated localizing matrices. */
        std::map<LocalizingMatrixIndex, ptrdiff_t> localizingMatrixIndices;

        /** The index (in this->matrices) of substituted matrices. */
        std::map<std::pair<ptrdiff_t, ptrdiff_t>, ptrdiff_t> substitutedMatrixIndices;


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
        [[nodiscard]] const class Matrix& MomentMatrix(size_t level) const;

        /**
         * Gets the localizing matrix for a particular sequence and hierarchy Level.
         * For thread safety, call for a read lock first.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @return The MomentMatrix for this particular Level.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class Matrix& LocalizingMatrix(const LocalizingMatrixIndex& lmi) const;

        /**
         * Gets a substituted matrix from a particular source and rulebook index.
         * For thread safety, call for a read lock first.
         * @param source_index The matrix that the substitutions are applied to.
         * @param rulebook_index The rules applied to the matrix.
         * @return The SubstitutedMatrix.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class Matrix& SubstitutedMatrix(size_t source_index, size_t rulebook_index) const;

        /**
         * Access matrix by subscript corresponding to order of creation.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const Matrix& operator[](size_t index) const;

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
         * Get type of matrix system
         */
        [[nodiscard]] virtual std::string system_type_name() const {
            return "Generic Matrix System";
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
          * @param mt_policy Is multithreaded creation used?
          * @return Pair: Matrix index and created MomentMatrix object reference.
          */
        std::pair<size_t, class Matrix&>
        create_moment_matrix(size_t level,
                             Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        /**
         * Constructs a localizing matrix for a particular Level on a particular word, or returns a pre-existing one.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param level The hierarchy depth.
         * @param word The word.
         * @param mt_policy Is multithreaded creation used?
         * @return Pair: Matrix index and created LocalizingMatrix object reference.
         */
        std::pair<size_t, class Matrix&>
        create_localizing_matrix(const LocalizingMatrixIndex& lmi,
                                 Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        /**
         * Clone a matrix, with substituted values, or returns pre-existing substituted matrix.
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         * @param matrix_index The ID of the matrix to clone.
         * @param rule_index The ID of the rulebook to apply.
         * @return Index
         */
        std::pair<size_t, class Matrix&>
        create_substituted_matrix(size_t matrix_index, size_t rule_index);

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
         * Check if a substituted matrix has been generated from a particular source matrix and rulebook.
         * For thread safety, call for a read lock first.
         * @param source_index The matrix that the substitutions are applied to.
         * @param rulebook_index The rules applied to the matrix..
         * @return The numerical index within this matrix system, or -1 if not found.
         */
        [[nodiscard]] ptrdiff_t find_substituted_matrix(size_t source_index, size_t rulebook_index) const noexcept;

        /**
         * Ensure that all symbols up to a particular length are defined in system, and mapped.
         * @return True if new symbols were created.
         */
        bool generate_dictionary(size_t word_length);

        /**
         * Import a list of moment substitution rules
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         */
        std::pair<size_t, MomentSubstitutionRulebook&>
        add_rulebook(std::unique_ptr<MomentSubstitutionRulebook> rulebook);

        /**
         * Import a list of moment substitution rules
         * Will lock until all read locks have expired - so do NOT first call for a read lock...!
         */
        std::pair<size_t, MomentSubstitutionRulebook&>
        merge_rulebooks(size_t existing_rulebook_id, MomentSubstitutionRulebook&& rulebook);

        /**
         * Get a list of moment substitution rules.
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] MomentSubstitutionRulebook& rulebook(size_t index);

        /**
         * Get a list of moment substitution rules
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] const MomentSubstitutionRulebook& rulebook(size_t index) const;

        /**
         * Counts number of rulebooks in system
         */
        [[nodiscard]] size_t rulebook_count() const noexcept { return this->rulebooks.size(); }

        /**
         * Gets a read (shared) lock for accessing data within the matrix system.
         */
        [[nodiscard]] auto get_read_lock() const {
            return std::shared_lock{this->rwMutex};
        }

        /**
         * Gets a write (exclusive) lock for manipulating data within the matrix system.
         */
        [[nodiscard]] auto get_write_lock() {
            return std::unique_lock{this->rwMutex};
        }

        /**
         * Gets the polynomial factory for this system.
         */
        [[nodiscard]] const PolynomialFactory& polynomial_factory() const noexcept {
            assert(this->poly_factory);
            return *this->poly_factory;
        }

    protected:
        /**
         * Replace polynomial factory with new factory.
         * Undefined behaviour if called after construction of matrix system.
         */
        void replace_polynomial_factory(std::unique_ptr<PolynomialFactory> new_factory) noexcept;

        /**
         * Overrideable method, called to generate a moment matrix.
         * @param level The moment matrix level.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new moment matrix.
         */
        virtual std::unique_ptr<class Matrix>
        createNewMomentMatrix(size_t level, Multithreading::MultiThreadPolicy mt_policy );

        /**
         * Virtual method, called to generate a localizing matrix.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new localizing matrix.
         */
        virtual std::unique_ptr<class Matrix>
        createNewLocalizingMatrix(const LocalizingMatrixIndex& lmi, Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Virtual method, called after a moment matrix is generated.
         * @param level The moment matrix level.
         * @param mm The newly generated moment matrix.
         */
        virtual void onNewMomentMatrixCreated(size_t level, const class Matrix& mm) { }

        /**
         * Virtual method, called after a localizing matrix is generated.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param lm The newly generated localizing matrix.
         */
        virtual void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex& lmi,
                                                  const class Matrix& lm) { }

       /**
        * Virtual method, called after a substituted matrix is generated.
        * @param source_index The source matrix index.
        * @param source The source matrix.
        * @param rulebook_index The rulebook index.
        * @param rulebook The rulebook.
        * @param subbed_matrix The newly created substituted matrix.
        */
        virtual void onNewSubstitutedMatrixCreated(size_t source_index, const class Matrix& source,
                                                   size_t rulebook_index, const MomentSubstitutionRulebook& rulebook,
                                                   const class Matrix& subbed_matrix) { }

        /**
         * Virtual method, called after a dictionary is generated.
         * @param word_length The dictionary word-length requested
         * @param osg The operator sequence generator.
         */
        virtual void onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator& osg) { }

        /**
         * Virtual method, called after a rulebook has been added
         * @param index The index the new rulebook
         * @param rulebook The rulebook itself.
         * @param insertion True if new addition, false if a merge.
         */
        virtual void onRulebookAdded(size_t index, const MomentSubstitutionRulebook& rb, bool insertion) { }

        /**
         * Get read-write access to symbolic matrix by index. Changes should not be made without a write lock.
         * @param index The number of the matrix within the system.
         * @return A reference to the requested matrix.
         * @throws errors::missing_component if index does not correspond to a matrix in the system.
         */
        Matrix& get(size_t index);

        /**
         * Add symbolic matrix to end of array. Changes should not be made without a write lock.
         * @param matrix Owning pointer to matrix to add.
         * @return The index of the newly appended matrix.
         */
        ptrdiff_t push_back(std::unique_ptr<Matrix> matrix);

    };
}
