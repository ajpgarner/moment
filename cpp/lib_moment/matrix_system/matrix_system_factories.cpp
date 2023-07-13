/**
 * matrix_system_factories.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_indices.h"
#include "matrix_system.h"
#include "matrix_system_factories.h"

#include "matrix/matrix.h"
#include "scenarios/context.h"
#include "symbolic/rules/moment_rulebook.h"

#include <memory>

namespace Moment {


    std::pair<ptrdiff_t, Matrix &>
    MomentMatrixFactory::operator()(const size_t level, const Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.createNewMomentMatrix(level, mt_policy));
        return std::pair<ptrdiff_t, Matrix&>(matrixIndex, *system.matrices.back());
    }

    void MomentMatrixFactory::notify(const size_t level, Matrix &matrix) {
        this->system.onNewMomentMatrixCreated(level, matrix);
    }

    std::string MomentMatrixFactory::not_found_msg(size_t level) const {
        std::stringstream errSS;
        errSS << "A moment matrix for level " << level << " has not yet been generated.";
        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> MomentMatrixFactory::get_write_lock() {
        return system.get_write_lock();
    }



    std::pair<ptrdiff_t, Matrix &> LocalizingMatrixFactory::operator()(const LocalizingMatrixIndex& lmi,
                                                                       Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.createNewLocalizingMatrix(lmi, mt_policy));
        return std::pair<ptrdiff_t, Matrix&>(matrixIndex, *system.matrices.back());
    }


    void LocalizingMatrixFactory::notify(const LocalizingMatrixIndex &lmi, Matrix &matrix) {
        this->system.onNewLocalizingMatrixCreated(lmi, matrix);
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

    std::pair<ptrdiff_t, Matrix &>
    PolynomialLocalizingMatrixFactory::operator()(const PolynomialLMIndex &index,
                                                  Multithreading::MultiThreadPolicy mt_policy) {
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(system.createNewPolyLM(index, mt_policy));
        return std::pair<ptrdiff_t, Matrix&>(matrixIndex, *system.matrices.back());
    }

    void PolynomialLocalizingMatrixFactory::notify(const PolynomialLMIndex &index, Matrix &matrix) {
        this->system.onNewPolyLMCreated(index, matrix);
    }

    std::string PolynomialLocalizingMatrixFactory::not_found_msg(const PolynomialLMIndex &pmi) const {
        std::stringstream errSS;
        errSS << "Localizing matrix of Level " << pmi.first
              << " for polynomial \"" << pmi.second.as_string_with_operators(system.Symbols(), false)
              << "\" has not yet been generated.";
        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> PolynomialLocalizingMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }


    std::pair<ptrdiff_t, Matrix &>
    SubstitutedMatrixFactory::operator()(const Index& index,
                                         Multithreading::MultiThreadPolicy mt_policy) {
        auto& source_matrix = system.get(index.first); // <- throws if not found!
        auto& rulebook = system.rulebook(index.second); // <- throws if not found!

        // Do creation
        const auto matrixIndex = static_cast<ptrdiff_t>(system.matrices.size());
        system.matrices.emplace_back(rulebook.create_substituted_matrix(*this->system.symbol_table, source_matrix));
        return {matrixIndex, *(system.matrices.back())};
    }

    void SubstitutedMatrixFactory::notify(const std::pair<ptrdiff_t, ptrdiff_t> &index, Matrix &matrix) {
        assert(index.first < system.matrices.size() && system.matrices[index.first]);
        assert(index.second < system.rulebooks.size() && system.rulebooks[index.second]);
        const auto& src_matrix = *system.matrices[index.first];
        const auto& rulebook = *system.rulebooks[index.second];
        system.onNewSubstitutedMatrixCreated(index.first, src_matrix, index.second, rulebook, matrix);
    }

    std::string SubstitutedMatrixFactory::not_found_msg(const std::pair<ptrdiff_t, ptrdiff_t> &index) const {
        const auto [rulebook_index, source_index] = index;

        std::stringstream errSS;
        if ((source_index < 0) || (source_index > system.matrices.size())) {
            errSS << "\nThe source matrix index " << source_index
                  << " is out of range, so the requested substituted matrix does not exist.";
        } else if ((rulebook_index < 0) || (rulebook_index > system.rulebooks.size())) {
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
    static_assert(makes_matrices<MomentMatrixFactory, Matrix, size_t>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<LocalizingMatrixFactory, Matrix, LocalizingMatrixIndex>);

    /** Ensure PolynomialLocalizingMatrixFactory meets concept. */
    static_assert(makes_matrices<PolynomialLocalizingMatrixFactory, Matrix, PolynomialLMIndex>);

    /** Ensure SubstitutedMatrixFactory meets concept. */
    static_assert(makes_matrices<SubstitutedMatrixFactory, Matrix, std::pair<ptrdiff_t, ptrdiff_t>>);

}