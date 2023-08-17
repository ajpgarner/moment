/**
 * matrix_factories.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_indices.h"
#include "matrix_system.h"
#include "matrix_factories.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "scenarios/context.h"
#include "symbolic/rules/moment_rulebook.h"

#include <memory>

namespace Moment {


    std::pair<ptrdiff_t, SymbolicMatrix &>
    MomentMatrixFactory::operator()(MaintainsMutex::WriteLock& lock, const size_t level,
                                    const Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.createNewMomentMatrix(lock, level, mt_policy));
        return std::pair<ptrdiff_t, SymbolicMatrix&>(matrixIndex, *system.matrices.back());
    }

    void MomentMatrixFactory::notify(const MaintainsMutex::WriteLock& lock, const size_t level, SymbolicMatrix &matrix) {
        this->system.onNewMomentMatrixCreated(lock, level, matrix);
    }

    std::string MomentMatrixFactory::not_found_msg(size_t level) const {
        std::stringstream errSS;
        errSS << "A moment matrix for level " << level << " has not yet been generated.";
        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> MomentMatrixFactory::get_write_lock() {
        return system.get_write_lock();
    }



    std::pair<ptrdiff_t, SymbolicMatrix &>
    LocalizingMatrixFactory::operator()(MaintainsMutex::WriteLock& lock, const LocalizingMatrixIndex& lmi,
                                        Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.createNewLocalizingMatrix(lock, lmi, mt_policy));
        return std::pair<ptrdiff_t, SymbolicMatrix&>(matrixIndex, *system.matrices.back());
    }


    void LocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                         const LocalizingMatrixIndex &lmi, SymbolicMatrix &matrix) {
        this->system.onNewLocalizingMatrixCreated(lock, lmi, matrix);
    }

    std::string LocalizingMatrixFactory::not_found_msg(const LocalizingMatrixIndex &lmi) const {
        std::stringstream errSS;
        errSS << "Localizing matrix of Level " << lmi.Level
              << " for sequence \"" << this->system.context->format_sequence(lmi.Word)
              << "\" has not yet been generated.";
        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> LocalizingMatrixFactory::get_write_lock() {
        return system.get_write_lock();
    }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialLocalizingMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                  const PolynomialLMIndex &index,
                                                  Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.createNewPolyLM(lock, index, mt_policy);
        PolynomialMatrix& matrixRef = *matrixPtr;
        system.matrices.emplace_back(std::move(matrixPtr));
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size()) - 1;
        return std::pair<ptrdiff_t, PolynomialMatrix&>(matrixIndex, matrixRef);
    }

    void PolynomialLocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                   const PolynomialLMIndex &index, PolynomialMatrix& matrix) {
        this->system.onNewPolyLMCreated(lock, index, matrix);
    }

    std::string PolynomialLocalizingMatrixFactory::not_found_msg(const PolynomialLMIndex &pmi) const {
        std::stringstream errSS;
        ContextualOS cErrSS{errSS, system.Context(), system.Symbols()};
        cErrSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cErrSS.format_info.show_braces = false;

        cErrSS << "Localizing matrix of Level " << pmi.Level
               << " for polynomial \"" << pmi.Polynomial
               << "\" has not yet been generated.";

        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> PolynomialLocalizingMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }


    std::pair<ptrdiff_t, SymbolicMatrix &>
    SubstitutedMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                         const Index& index,
                                         const Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(lock));
        auto& source_matrix = system.get(index.first); // <- throws if not found!
        auto& rulebook = system.Rulebook(index.second); // <- throws if not found!

        // Do creation
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(rulebook.create_substituted_matrix(*this->system.symbol_table, source_matrix,
                                                                        mt_policy));
        return {matrixIndex, *(system.matrices.back())};
    }

    void SubstitutedMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const std::pair<ptrdiff_t, ptrdiff_t> &index, SymbolicMatrix &matrix) {
        assert(index.first < system.matrices.size() && system.matrices[index.first]);
        assert(system.Rulebook.contains(index.second));
        const auto& src_matrix = *(system.matrices[index.first]);
        const auto& rulebook = system.Rulebook(index.second);
        system.onNewSubstitutedMatrixCreated(lock, index.first, src_matrix, index.second, rulebook, matrix);
    }

    std::string SubstitutedMatrixFactory::not_found_msg(const std::pair<ptrdiff_t, ptrdiff_t> &index) const {
        const auto [rulebook_index, source_index] = index;

        std::stringstream errSS;
        if ((source_index < 0) || (source_index > system.matrices.size())) {
            errSS << "\nThe source matrix index " << source_index
                  << " is out of range, so the requested substituted matrix does not exist.";
        } else if ((rulebook_index < 0) || (rulebook_index >= system.Rulebook.size())) {
            errSS << "\nThe rulebook index " << rulebook_index
                  << " is out of range, so the requested substituted matrix does not exist.";
        } else {
            errSS << "A substituted matrix formed by applying rule book " << rulebook_index
                  << " to matrix index " << source_index << " has not yet been generated.";
        }
        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> SubstitutedMatrixFactory::get_write_lock() {
        return system.get_write_lock();
    }

    /** Ensure MomentMatrixFactory meets concept. */
    static_assert(makes_matrices<MomentMatrixFactory, SymbolicMatrix, size_t>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<LocalizingMatrixFactory, SymbolicMatrix, LocalizingMatrixIndex>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<PolynomialLocalizingMatrixFactory, PolynomialMatrix, PolynomialLMIndex>);

    /** Ensure SubstitutedMatrixFactory meets concept. */
    static_assert(makes_matrices<SubstitutedMatrixFactory, SymbolicMatrix, std::pair<ptrdiff_t, ptrdiff_t>>);

}