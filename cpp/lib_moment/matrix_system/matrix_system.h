/**
 * matrix_system.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 *
 * FOR THREAD SAFETY: Functions accessing a matrix system should call for the read lock before accessing anything.
 */
#pragma once

#include "matrix_system_errors.h"
#include "standard_matrix_indices.h"
#include "rulebook_storage.h"

#include "dictionary/raw_polynomial.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

namespace Moment {

    class Context;
    class Dictionary;
    class SymbolicMatrix;
    class MomentRulebook;
    class OperatorSequenceGenerator;
    class PolynomialFactory;
    class RawPolynomial;
    class SymbolTable;

    /**
     * Base class for systems of operators, and their associated moment/localizing matrices.
     *
     * FOR THREAD SAFETY: Functions accessing a matrix system should call for the read lock before accessing anything.
     */
    class MatrixSystem : public MaintainsMutex {
    private:
        /** The operator context. */
        std::unique_ptr<class Context> context;

        /** Map from symbols to operator sequences, and real/imaginary indices. */
        std::unique_ptr<SymbolTable> symbol_table;

        /** Factory object for constructing polynomials. */
        std::unique_ptr<PolynomialFactory> poly_factory;

        /** List of matrices in the system. */
        std::vector<std::unique_ptr<SymbolicMatrix>> matrices;

    public:
        /** Indexed moment matrices. */
        MomentMatrixIndices MomentMatrix;

        /** Indexed localizing matrices.  */
        LocalizingMatrixIndices LocalizingMatrix;

        /** Indexed polynomial localizing matrices. */
        PolynomialLMIndices PolynomialLocalizingMatrix;

        /** Indexed substituted matrices. */
        SubstitutedMatrixIndices SubstitutedMatrix;

        /** Moment substitution rulebooks. */
        RulebookStorage Rulebook;

    public:
         /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         * @param zero_tolerance The multiplier of epsilon below which doubles are treated as zero.
         */
        explicit MatrixSystem(std::unique_ptr<class Context> context, double tolerance = 1.0);

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
         * Access matrix by subscript corresponding to order of creation.
         * For thread safety, call for a read lock first.
         * @throws errors::missing_component If index is invalid.
         */
        [[nodiscard]] const SymbolicMatrix& operator[](ptrdiff_t index) const;

        /**
          * Get read-write access to symbolic matrix by its index.
          * Changes should not be made without a write lock.
          * @param index The number of the matrix within the system.
          * @return A reference to the requested matrix.
          * @throws errors::missing_component if index does not correspond to a matrix in the system.
          */
        [[nodiscard]] SymbolicMatrix& get(ptrdiff_t index);

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
         * Gets the number of sequences in an OSG of a given length.
         * Returns 0 if system does not define operators.
         * NB: This could trigger the generation of a dictionary if one is not already known.
         */
        [[nodiscard]] size_t osg_size(size_t level) const;

        /**
         * Get type of matrix system
         */
        [[nodiscard]] virtual std::string system_type_name() const {
            return "Generic Matrix System";
        }

        /**
         * Ensure that all symbols up to a particular length are defined in system, and mapped.
         * @return True if new symbols were created.
         */
        bool generate_dictionary(size_t word_length);

        /**
         * Gets the polynomial factory for this system.
         */
        [[nodiscard]] const PolynomialFactory& polynomial_factory() const noexcept {
            assert(this->poly_factory);
            return *this->poly_factory;
        }

        /**
         * Add symbolic matrix to end of array. Changes should not be made without a write lock.
         * @param lock The write lock to the matrix system.
         * @param matrix Owning pointer to matrix to add.
         * @return The index of the newly appended matrix.
         */
        [[nodiscard]] ptrdiff_t push_back(const WriteLock& lock, std::unique_ptr<SymbolicMatrix> matrix);

    protected:
        /**
         * Replace polynomial factory with new factory.
         * Undefined behaviour if called after construction of matrix system.
         */
        void replace_polynomial_factory(std::unique_ptr<PolynomialFactory> new_factory);

        /**
         * Overrideable method, called to generate a moment matrix.
         * @param level The moment matrix level.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new moment matrix.
         */
        virtual std::unique_ptr<class SymbolicMatrix>
        create_moment_matrix(const WriteLock& lock, size_t level,
                             Multithreading::MultiThreadPolicy mt_policy );

        /**
         * Virtual method, called to generate a localizing matrix.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new localizing matrix.
         */
        virtual std::unique_ptr<class SymbolicMatrix>
        create_localizing_matrix(const WriteLock& lock, const LocalizingMatrixIndex& lmi,
                                 Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Virtual method, called to generate a polynomial localizing matrix matrix.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new localizing matrix.
         */
        virtual std::unique_ptr<class PolynomialMatrix>
        create_polynomial_localizing_matrix(const MaintainsMutex::WriteLock &lock,
                                            const PolynomialLocalizingMatrixIndex& index,
                                            Multithreading::MultiThreadPolicy mt_policy);

    public:
        /**
         * Special creation request for polynomial localizing matrix without well-defined Polynomial index.
         * @param level The hierarchy level of the localizing matrix.
         * @param raw_poly The raw polynomial object of the matrix.
         * @param mt_policy Is multithreaded creation used?
         * @return Index and reference to new localizing matrix.
         */
        std::pair<ptrdiff_t, const Moment::PolynomialMatrix&>
        create_and_register_localizing_matrix(size_t level, const RawPolynomial& raw_poly,
                                              Multithreading::MultiThreadPolicy mt_policy);

    protected:
        /**
         * Virtual method, called to expand a rulebook according to matrix system extra rules.
         * @param rulebook The rulebook to expand
         * @param from_symbol The largest symbol the rulebook previously had knowledge about.
         * @return Number of new rules added.
         */
        virtual ptrdiff_t expand_rulebook(MomentRulebook& rulebook, size_t from_symbol) { return 0; }


        /**
         * Virtual method, called after a moment matrix is generated.
         * @param level The moment matrix level.
         * @param mm The newly generated moment matrix.
         */
        virtual void on_new_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                          size_t level,
                                          ptrdiff_t matrix_offset, const class SymbolicMatrix& mm) { }

        /**
         * Virtual method, called after a (flat monomial) localizing matrix is generated.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param lm The newly generated localizing matrix.
         */
        virtual void on_new_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                              const LocalizingMatrixIndex& lmi,
                                              ptrdiff_t matrix_offset,
                                              const class SymbolicMatrix& lm) { }

        /**
         * Virtual method, called after a polynomial localizing matrix is generated.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param lm The newly generated localizing matrix.
         */
        virtual void on_new_polynomial_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                         const PolynomialLocalizingMatrixIndex& plm_index,
                                                         ptrdiff_t matrix_offset, const class PolynomialMatrix& plm) { }

       /**
        * Virtual method, called after a substituted matrix is generated.
        * @param source_index The source matrix index.
        * @param source The source matrix.
        * @param rulebook_index The rulebook index.
        * @param rulebook The rulebook.
        * @param subbed_matrix The newly created substituted matrix.
        */
        virtual void on_new_substituted_matrix(const MaintainsMutex::WriteLock& write_lock,
                                               size_t source_index, const class SymbolicMatrix& source,
                                               size_t rulebook_index, const MomentRulebook& rulebook,
                                               ptrdiff_t matrix_offset, const class SymbolicMatrix& subbed_matrix) { }

        /**
         * Virtual method, called after a dictionary is generated.
         * @param word_length The dictionary word-length requested
         * @param osg The operator sequence generator.
         */
        virtual void on_new_dictionary(const MaintainsMutex::WriteLock& write_lock,
                                       size_t word_length, const OperatorSequenceGenerator& osg) { }

        /**
         * Virtual method, called after a rulebook has been added or merged
         * @param index The index the new rulebook
         * @param rulebook The rulebook itself.
         * @param insertion True if new addition, false if a merge.
         */
        virtual void on_rulebook_added(const MaintainsMutex::WriteLock& write_lock,
                                       size_t index, const MomentRulebook& rb, bool insertion) { }

        /**
         * Virtual method, called after new symbols have been added to the symbol table.
         * @param old_symbol_count The number of symbols before the update.
         * @param new_symbol_count The number of symbols after the update.
         */
        virtual void on_new_symbols_registered(const MaintainsMutex::WriteLock& write_lock,
                                               size_t old_symbol_count, size_t new_symbol_count) { }


    public:
        friend RulebookStorage;
        friend MomentMatrixFactory;
        friend LocalizingMatrixFactory;
        friend PolynomialLocalizingMatrixFactory;
        friend SubstitutedMatrixFactory;

        /** Matrix index classes. */
        template<typename matrix_t, typename index_t, stores_indices<index_t> index_storage_t,
                makes_matrices<matrix_t, index_t> factory_t,
                typename matrix_system_t>
        friend class MatrixIndices;

    };
}
