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

#include "symbolic/substitution_list.h"
#include "symbolic/symbol_table.h"

#include "scenarios/context.h"
#include "scenarios/word_list.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Moment {
    namespace {
        const Context& assertContext(const std::unique_ptr<Context>& ctxtIn) {
            assert(ctxtIn);
            return *ctxtIn;
        }
    }

    MatrixSystem::MatrixSystem(std::unique_ptr<class Context> ctxtIn)
        : context{std::move(ctxtIn)}, symbol_table{std::make_unique<SymbolTable>(assertContext(context))} {
    }

    MatrixSystem::~MatrixSystem() noexcept = default;

    ptrdiff_t MatrixSystem::highest_moment_matrix() const noexcept {
        if (this->momentMatrixIndices.empty()) {
            return -1;
        }
        return static_cast<ptrdiff_t>(this->momentMatrixIndices.size()) - 1;
    }

    const Matrix& MatrixSystem::MomentMatrix(size_t level) const {
        auto index = this->find_moment_matrix(level);
        if (index < 0) {
            throw errors::missing_component("Moment matrix of Level " + std::to_string(level) + " not yet generated.");
        }
        return *matrices[index];
    }

    std::pair<size_t, class Matrix&> MatrixSystem::create_moment_matrix(size_t level) {
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
        this->matrices.emplace_back(this->createNewMomentMatrix(level));
        this->momentMatrixIndices[level] = matrixIndex;

        auto& output = *this->matrices[matrixIndex];

        // Delegated post-generation
        this->onNewMomentMatrixCreated(level, output);

        return {matrixIndex, output};
    }


    const Matrix& MatrixSystem::LocalizingMatrix(const LocalizingMatrixIndex& lmi) const {
        ptrdiff_t index = this->find_localizing_matrix(lmi);

        if (index <= 0) {
            throw errors::missing_component("Localizing matrix of Level " + std::to_string(lmi.Level)
                                            + " for sequence \"" + this->context->format_sequence(lmi.Word)
                                            + "\" not yet been generated.");
        }

        return *matrices[momentMatrixIndices[index]];
    }

    std::pair<size_t, class Matrix&>
    MatrixSystem::create_localizing_matrix(const LocalizingMatrixIndex& lmi) {
        // Call for write lock...
        auto lock = this->get_write_lock();

        // First, try read...
        ptrdiff_t index = this->find_localizing_matrix(lmi);
        if (index >= 0) {
            return {index, *matrices[index]};
        }

        // Otherwise,generate new localizing matrix, and insert index
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(this->createNewLocalizingMatrix(lmi));
        this->localizingMatrixIndices.emplace(std::make_pair(lmi, matrixIndex));

        // Get reference to new matrix, and call derived classes...
        auto& newLM = (*this->matrices.back());
        this->onNewLocalizingMatrixCreated(lmi, newLM);

        // Return (reference to) matrix just added
        return {matrixIndex, newLM};
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

        return where->second;
    }

    std::unique_ptr<class Matrix> MatrixSystem::createNewMomentMatrix(const size_t level) {
        auto operator_matrix = std::make_unique<class MomentMatrix>(*this->context, level);
        return std::make_unique<MonomialMatrix>(*this->symbol_table, std::move(operator_matrix));
    }


    std::unique_ptr<class Matrix> MatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex& lmi) {
        auto operator_matrix = std::make_unique<class LocalizingMatrix>(*this->context, lmi);
        return std::make_unique<MonomialMatrix>(*this->symbol_table, std::move(operator_matrix));
    }



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

    std::pair<size_t, class Matrix &>
    MatrixSystem::clone_and_substitute(size_t matrix_index, std::unique_ptr<SubstitutionList> list) {
        assert(list);

        // Get write lock before pushing matrix
        auto write_lock = this->get_write_lock();

        auto& source_matrix = this->get(matrix_index);
        size_t new_index = this->matrices.size();

        if (source_matrix.is_monomial()) {
            const auto& mono_source = dynamic_cast<MonomialMatrix&>(source_matrix);
            this->matrices.emplace_back(std::make_unique<SubstitutedMatrix>(*this->context, *this->symbol_table,
                                                                            mono_source, std::move(list)));
        } else {
            const auto& poly_source = dynamic_cast<PolynomialMatrix&>(source_matrix);
            throw std::runtime_error{"Substitution into polynomial symbol matrices has not yet been implemented."};
        }

        auto& new_matrix = *(this->matrices.back());
        return {new_index, new_matrix};
    }

    bool MatrixSystem::generate_dictionary(const size_t word_length) {
        auto write_lock = this->get_write_lock();

        auto [osg_size, new_symbols] = this->symbol_table->fill_to_word_length(word_length);

        this->onDictionaryGenerated(word_length, this->context->operator_sequence_generator(word_length));

        return new_symbols;
    }
}