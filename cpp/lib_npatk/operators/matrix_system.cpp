/**
 * matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_system.h"

#include "matrix/symbol_table.h"
#include "matrix/localizing_matrix.h"
#include "matrix/moment_matrix.h"

#include "context.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace NPATK {
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
        return static_cast<ptrdiff_t>(this->momentMatrixIndices.size()) - 1;
    }


    const MomentMatrix &MatrixSystem::MomentMatrix(size_t level) const {
        auto index = this->find_moment_matrix(level);
        if (index < 0) {
            throw errors::missing_component("Moment matrix of Level " + std::to_string(level) + " not yet generated.");
        }
        return dynamic_cast<const class MomentMatrix&>(*matrices[index]);
    }

    std::pair<size_t, class MomentMatrix&> MatrixSystem::create_moment_matrix(size_t level) {
        // Call for write lock...
        auto lock = this->get_write_lock();

        // First, try read
        auto index = this->find_moment_matrix(level);
        if (index >= 0) {
            return {index, dynamic_cast<class MomentMatrix&>(*matrices[index])};
        }

        // Delegated pre-generation
        this->beforeNewMomentMatrixCreated(level);

        // Fill with null elements if some are missing
        if (this->momentMatrixIndices.size() < level+1) {
            this->momentMatrixIndices.resize(level+1, -1);
        }

        // Generate new moment matrix
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::make_unique<class MomentMatrix>(*this->context, *this->symbol_table, level));
        this->momentMatrixIndices[level] = matrixIndex;

        auto& output = dynamic_cast<class MomentMatrix&>(*this->matrices[matrixIndex]);

        // Delegated post-generation
        this->onNewMomentMatrixCreated(level, output);

        return {matrixIndex, output};
    }


    const LocalizingMatrix& MatrixSystem::LocalizingMatrix(const LocalizingMatrixIndex& lmi) const {
        ptrdiff_t index = this->find_localizing_matrix(lmi);

        if (index <= 0) {
            throw errors::missing_component("Localizing matrix of Level " + std::to_string(lmi.Level)
                                            + " for sequence \"" + this->context->format_sequence(lmi.Word)
                                            + "\" not yet been generated.");
        }

        return dynamic_cast<const class LocalizingMatrix&>(*matrices[momentMatrixIndices[index]]);
    }

    std::pair<size_t, class LocalizingMatrix&>
    MatrixSystem::create_localizing_matrix(const LocalizingMatrixIndex& lmi) {
        // Call for write lock...
        auto lock = this->get_write_lock();

        // First, try read...
        ptrdiff_t index = this->find_localizing_matrix(lmi);
        if (index >= 0) {
            return {index, dynamic_cast<class LocalizingMatrix&>(*matrices[momentMatrixIndices[index]])};
        }

        // Delegated pre-generation
        this->beforeNewLocalizingMatrixCreated(lmi);

        // Otherwise,generate new localizing matrix, and insert index
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::make_unique<class LocalizingMatrix>(*this->context, *this->symbol_table, lmi));
        this->localizingMatrixIndices.emplace(std::make_pair(lmi, matrixIndex));

        // Get reference to new matrix, and call derived classes...
        auto& newLM = dynamic_cast<class LocalizingMatrix&>(*this->matrices.back());
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
}