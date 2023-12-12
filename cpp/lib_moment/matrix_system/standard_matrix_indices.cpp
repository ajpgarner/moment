/**
 * standard_matrix_factories.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_indices.h"
#include "matrix_system.h"
#include "standard_matrix_indices.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "scenarios/context.h"
#include "symbolic/rules/moment_rulebook.h"

#include <memory>

namespace Moment {

    std::pair<ptrdiff_t, SymbolicMatrix &>
    MomentMatrixFactory::operator()(MaintainsMutex::WriteLock& lock, const Index level,
                                    const Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.create_moment_matrix(lock, level, mt_policy));
        return std::pair<ptrdiff_t, SymbolicMatrix&>(matrixIndex, *system.matrices.back());
    }

    void MomentMatrixFactory::notify(const MaintainsMutex::WriteLock& lock, const Index level,
                                     ptrdiff_t offset,
                                     SymbolicMatrix &matrix) {
        this->system.on_new_moment_matrix(lock, level, offset, matrix);
    }

    std::pair<ptrdiff_t, SymbolicMatrix &>
    LocalizingMatrixFactory::operator()(MaintainsMutex::WriteLock& lock, const LocalizingMatrixIndex& lmi,
                                        Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.create_localizing_matrix(lock, lmi, mt_policy));
        return std::pair<ptrdiff_t, SymbolicMatrix&>(matrixIndex, *system.matrices.back());
    }


    void LocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                         const LocalizingMatrixIndex &lmi,
                                         ptrdiff_t offset, SymbolicMatrix &matrix) {
        this->system.on_new_localizing_matrix(lock, lmi, offset, matrix);
    }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialLocalizingMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                  const PolynomialLMIndex &index,
                                                  Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_polynomial_localizing_matrix(lock, index, mt_policy);
        PolynomialMatrix& matrixRef = *matrixPtr;
        system.matrices.emplace_back(std::move(matrixPtr));
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size()) - 1;
        return std::pair<ptrdiff_t, PolynomialMatrix&>(matrixIndex, matrixRef);
    }

    void PolynomialLocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                   const PolynomialLMIndex &index,
                                                   ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_polynomial_localizing_matrix(lock, index, offset, matrix);
    }

    std::pair<ptrdiff_t, SymbolicMatrix &>
    SubstitutedMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                         const Index& index,
                                         const Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(lock));
        auto& source_matrix = system.get(index.SourceMatrix); // <- throws if not found!
        auto& rulebook = system.Rulebook(index.Rulebook); // <- throws if not found!

        // Do creation
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(rulebook.create_substituted_matrix(*this->system.symbol_table, source_matrix,
                                                                        mt_policy));
        return {matrixIndex, *(system.matrices.back())};
    }

    void SubstitutedMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const Index& index,
                                          ptrdiff_t offset, SymbolicMatrix& matrix) {
        assert(index.SourceMatrix < system.matrices.size() && system.matrices[index.SourceMatrix]);
        assert(system.Rulebook.contains(index.Rulebook));
        const auto& src_matrix = *(system.matrices[index.SourceMatrix]);
        const auto& rulebook = system.Rulebook(index.Rulebook);
        system.on_new_substituted_matrix(lock, index.SourceMatrix, src_matrix,
                                         index.Rulebook, rulebook, offset, matrix);
    }

    /** Ensure MomentMatrixFactory meets concept. */
    static_assert(makes_matrices<MomentMatrixFactory, SymbolicMatrix, MomentMatrixIndex>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<LocalizingMatrixFactory, SymbolicMatrix, LocalizingMatrixIndex>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<PolynomialLocalizingMatrixFactory, PolynomialMatrix, PolynomialLMIndex>);

    /** Ensure SubstitutedMatrixFactory meets concept. */
    static_assert(makes_matrices<SubstitutedMatrixFactory, SymbolicMatrix, SubstitutedMatrixIndex>);

}