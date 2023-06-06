/**
 * matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "matrix_system.h"

#include "matrix/localizing_matrix.h"
#include "matrix/moment_matrix.h"
#include "matrix/substituted_matrix.h"

#include "symbolic/moment_substitution_rulebook.h"
#include "symbolic/order_symbols_by_hash.h"
#include "symbolic/symbol_table.h"

#include "scenarios/context.h"
#include "scenarios/word_list.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Moment {
    namespace {
        const Context& assertContext(const std::unique_ptr<Context>& contextIn) {
            assert(contextIn);
            return *contextIn;
        }
    }

    MatrixSystem::MatrixSystem(std::unique_ptr<class Context> ctxtIn)
        : context{std::move(ctxtIn)}, symbol_table{std::make_unique<SymbolTable>(assertContext(context))} {
    }

    MatrixSystem::~MatrixSystem() noexcept = default;

    const Matrix &MatrixSystem::operator[](size_t index) const {
        if (index >= this->matrices.size()) {
            throw errors::missing_component("Matrix index out of range.");
        }
        if (!this->matrices[index]) {
            throw errors::missing_component("Matrix at supplied index was missing.");
        }
        return *this->matrices[index];
    }

    Matrix& MatrixSystem::get(size_t index) {
        if (index >= this->matrices.size()) {
            throw errors::missing_component("Matrix index out of range.");
        }
        if (!this->matrices[index]) {
            throw errors::missing_component("Matrix at supplied index was missing.");
        }
        return *this->matrices[index];
    }

    ptrdiff_t MatrixSystem::push_back(std::unique_ptr<Matrix> matrix) {
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::move(matrix));
        return matrixIndex;
    }



    const Matrix& MatrixSystem::MomentMatrix(size_t level) const {
        auto index = this->find_moment_matrix(level);
        if (index < 0) {
            std::stringstream errSS;
            errSS << "Moment matrix of Level " << std::to_string(level) << " not yet been generated.";
            throw errors::missing_component(errSS.str());
        }
        return *matrices[index];
    }

    const Matrix& MatrixSystem::LocalizingMatrix(const LocalizingMatrixIndex& lmi) const {
        ptrdiff_t index = this->find_localizing_matrix(lmi);

        if (index <= 0) {
            std::stringstream errSS;
            errSS << "Localizing matrix of Level " << lmi.Level
                    << " for sequence \"" << this->context->format_sequence(lmi.Word)
                    << "\" has not yet been generated.";
                  throw errors::missing_component(errSS.str());
        }

        return *matrices[index];
    }

    const class Matrix &MatrixSystem::SubstitutedMatrix(size_t source_index, size_t rulebook_index) const {
        ptrdiff_t index = this->find_substituted_matrix(source_index, rulebook_index);
        if (index <= 0) {
            std::stringstream errSS;
            errSS << "A substituted matrix formed by applying rule book " << rulebook_index
                  << " to matrix index " << source_index << " has not yet been generated.";
            throw errors::missing_component(errSS.str());
        }

        return *matrices[index];
    }

    std::pair<size_t, class Matrix&>
    MatrixSystem::create_moment_matrix(const size_t level,
                                       const Multithreading::MultiThreadPolicy mt_policy) {
        // Call for write lock...
        auto lock = this->get_write_lock();

        // First, try read
        auto index = this->find_moment_matrix(level);
        if (index >= 0) {
            return {index, *matrices[index]};
        }

        // Fill with null elements if some are missing
        if (this->momentMatrixIndices.size() < level+1) {
            this->momentMatrixIndices.resize(level+1, -1);
        }

        // Generate new moment matrix
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(this->createNewMomentMatrix(level, mt_policy));
        this->momentMatrixIndices[level] = matrixIndex;

        auto& output = *this->matrices[matrixIndex];

        // Delegated post-generation
        this->onNewMomentMatrixCreated(level, output);

        return {matrixIndex, output};
    }

    std::pair<size_t, class Matrix&>
    MatrixSystem::create_localizing_matrix(const LocalizingMatrixIndex& lmi,
                                           const Multithreading::MultiThreadPolicy mt_policy) {
        // Call for write lock...
        auto lock = this->get_write_lock();

        // First, try read...
        ptrdiff_t index = this->find_localizing_matrix(lmi);
        if (index >= 0) {
            return {index, *matrices[index]};
        }

        // Otherwise,generate new localizing matrix, and insert index
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(this->createNewLocalizingMatrix(lmi, mt_policy));
        this->localizingMatrixIndices.emplace(std::make_pair(lmi, matrixIndex));

        // Get reference to new matrix, and call derived classes...
        auto& newLM = (*this->matrices.back());
        this->onNewLocalizingMatrixCreated(lmi, newLM);

        // Return (reference to) matrix just added
        return {matrixIndex, newLM};
    }


    std::pair<size_t, class Matrix &>
    MatrixSystem::create_substituted_matrix(const size_t matrix_index, const size_t rulebook_index) {
        // Get write lock
        auto write_lock = this->get_write_lock();

        // First, try read...
        ptrdiff_t index = this->find_substituted_matrix(matrix_index, rulebook_index);
        if (index >= 0) {
            return {index, *matrices[index]};
        }

        // Source matrix
        const auto& source_matrix = this->get(matrix_index);

        // MonomialRules
        const auto& rulebook = this->rulebook(rulebook_index);

        // Make reduced matrix
        this->matrices.emplace_back(rulebook.create_substituted_matrix(*this->symbol_table, source_matrix));
        size_t new_index = this->matrices.size() - 1;
        auto& new_matrix = *(this->matrices.back());

        // Add index
        const auto index_key = std::make_pair(static_cast<ptrdiff_t>(matrix_index),
                                              static_cast<ptrdiff_t>(rulebook_index));
        this->substitutedMatrixIndices.emplace(std::make_pair(index_key, static_cast<ptrdiff_t>(new_index)));

        // Call derived-class post-processing
        this->onNewSubstitutedMatrixCreated(matrix_index, source_matrix,
                                            rulebook_index, rulebook, new_matrix);

        // Return matrix
        return {new_index, new_matrix};
    }


    ptrdiff_t MatrixSystem::highest_moment_matrix() const noexcept {
        if (this->momentMatrixIndices.empty()) {
            return -1;
        }
        return static_cast<ptrdiff_t>(this->momentMatrixIndices.size()) - 1;
    }

    ptrdiff_t MatrixSystem::find_moment_matrix(size_t level) const noexcept {
        // Do our indices even extend this far?
        if (level >= momentMatrixIndices.size()) {
            return -1;
        }

        // Is index set, positive and in bounds?
        auto mmIndex = momentMatrixIndices[level];
        if ((mmIndex >= this->matrices.size()) || (mmIndex < 0)) {
            return -1;
        }

        // Is matrix null?
        if (!this->matrices[mmIndex]) {
            return -1;
        }

        // Otherwise, return index
        return mmIndex;
    }

    ptrdiff_t MatrixSystem::find_localizing_matrix(const LocalizingMatrixIndex& lmi) const noexcept {
        auto where = this->localizingMatrixIndices.find(lmi);
        if (where == this->localizingMatrixIndices.end()) {
            return -1;
        }

        if (!this->matrices[where->second]) {
            return -1;
        }

        return where->second;
    }

    ptrdiff_t MatrixSystem::find_substituted_matrix(const size_t source_index,
                                                    const size_t rulebook_index) const noexcept {
        const auto index_key = std::make_pair(static_cast<ptrdiff_t>(source_index),
                                              static_cast<ptrdiff_t>(rulebook_index));
        auto where = this->substitutedMatrixIndices.find(index_key);
        if (where == this->substitutedMatrixIndices.end()) {
            return -1;
        }

        if (!this->matrices[where->second]) {
            return -1;
        }

        return where->second;
    }

    std::unique_ptr<class Matrix>
    MatrixSystem::createNewMomentMatrix(const size_t level, const Multithreading::MultiThreadPolicy mt_policy) {
        auto operator_matrix = std::make_unique<class MomentMatrix>(*this->context, level, mt_policy);
        return std::make_unique<MonomialMatrix>(*this->symbol_table, std::move(operator_matrix));
    }


    std::unique_ptr<class Matrix>
    MatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex& lmi,
                                            const Multithreading::MultiThreadPolicy mt_policy) {
        auto operator_matrix = std::make_unique<class LocalizingMatrix>(*this->context, lmi, mt_policy);
        return std::make_unique<MonomialMatrix>(*this->symbol_table, std::move(operator_matrix));
    }

    bool MatrixSystem::generate_dictionary(const size_t word_length) {
        auto write_lock = this->get_write_lock();

        auto [osg_size, new_symbols] = this->symbol_table->fill_to_word_length(word_length);

        this->onDictionaryGenerated(word_length, this->context->operator_sequence_generator(word_length));

        return new_symbols;
    }

    std::pair<size_t, MomentSubstitutionRulebook&>
    MatrixSystem::add_rulebook(std::unique_ptr<MomentSubstitutionRulebook> input_rulebook_ptr) {
        auto write_lock = this->get_write_lock();
        assert(&input_rulebook_ptr->symbols == this->symbol_table.get());

        this->rulebooks.emplace_back(std::move(input_rulebook_ptr));

        // Get info
        size_t rulebook_index = this->rulebooks.size()-1;
        auto& rulebook = *this->rulebooks[rulebook_index];

        // Check if rulebook has a name, set default name otherwise.
        if (rulebook.name().empty()) {
            std::stringstream nameSS;
            nameSS << "Rulebook #" << rulebook_index;
            rulebook.set_name(nameSS.str());
        }

        // Dispatch notification to derived classes
        this->onRulebookAdded(rulebook_index, rulebook, true);

        // Report creation
        return {rulebook_index, rulebook};
    }

    std::pair<size_t, MomentSubstitutionRulebook &>
    MatrixSystem::merge_rulebooks(const size_t existing_rulebook_id, MomentSubstitutionRulebook&& input_rulebook) {
        auto write_lock = this->get_write_lock();

        auto& existing_rulebook = this->rulebook(existing_rulebook_id);

        existing_rulebook.combine_and_complete(std::move(input_rulebook));

        // NB: Name should be handled already, either from existing name, or newly-merged-in name.

        // Dispatch notification of merge-in to derived classes
        this->onRulebookAdded(existing_rulebook_id, existing_rulebook, false);

        // Report merge
        return {existing_rulebook_id, existing_rulebook};
    }

    MomentSubstitutionRulebook& MatrixSystem::rulebook(size_t index) {
        if (index >= this->rulebooks.size()) {
            throw errors::missing_component("Rulebook index out of range.");
        }
        if (!this->rulebooks[index]) {
            throw errors::missing_component("Rulebook at supplied index was missing.");
        }
        return *this->rulebooks[index];
    }

    const MomentSubstitutionRulebook& MatrixSystem::rulebook(size_t index) const {
        if (index >= this->rulebooks.size()) {
            throw errors::missing_component("Rulebook index out of range.");
        }
        if (!this->rulebooks[index]) {
            throw errors::missing_component("Rulebook at supplied index was missing.");
        }
        return *this->rulebooks[index];
    }

}