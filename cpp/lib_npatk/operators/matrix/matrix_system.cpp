/**
 * matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_system.h"

#include "symbol_table.h"
#include "operator_matrix.h"
#include "localizing_matrix.h"
#include "moment_matrix.h"

#include "../context.h"

#include <algorithm>
#include <memory>
#include <mutex>
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


    bool MatrixSystem::hasMomentMatrix(size_t level) const noexcept {
        // Do our indices even extend this far?
        if (level >= momentMatrixIndices.size()) {
            return false;
        }

        // Is index set, positive and in bounds?
        auto mmIndex = momentMatrixIndices[level];
        if ((mmIndex >= this->matrices.size()) || (mmIndex < 0)) {
            return false;
        }

        // Is matrix not-null?
        return static_cast<bool>(this->matrices[mmIndex]);
    }

    ptrdiff_t MatrixSystem::highestMomentMatrix() const noexcept {
        return static_cast<ptrdiff_t>(this->momentMatrixIndices.size()) - 1;
    }


    const MomentMatrix &MatrixSystem::MomentMatrix(size_t level) const {
        if (!this->hasMomentMatrix(level)) {
            throw errors::missing_component("Moment matrix of Level " + std::to_string(level) + " not yet generated.");
        }
        return dynamic_cast<const class MomentMatrix&>(*matrices[momentMatrixIndices[level]]);
    }

    MomentMatrix& MatrixSystem::CreateMomentMatrix(size_t level) {
        // Call for write lock...
        auto lock = this->getWriteLock();

        // First, try read
        if (this->hasMomentMatrix(level)) {
            return dynamic_cast<class MomentMatrix&>(*matrices[momentMatrixIndices[level]]);
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

        return output;
    }


    ptrdiff_t MatrixSystem::localizingMatrixIndex(const LocalizingMatrixIndex& lmi) const noexcept {
        auto where = this->localizingMatrixIndices.find(lmi);
        if (where == this->localizingMatrixIndices.end()) {
            return -1;
        }

        return where->second;
    }

    const LocalizingMatrix& MatrixSystem::LocalizingMatrix(const LocalizingMatrixIndex& lmi) const {
        ptrdiff_t index = this->localizingMatrixIndex(lmi);

        if (index <= 0) {
            throw errors::missing_component("Localizing matrix of Level " + std::to_string(lmi.Level)
                                            + " for sequence \"" + this->context->format_sequence(lmi.Word)
                                            + "\" not yet been generated.");
        }

        return dynamic_cast<const class LocalizingMatrix&>(*matrices[momentMatrixIndices[index]]);
    }

    LocalizingMatrix& MatrixSystem::CreateLocalizingMatrix(const LocalizingMatrixIndex& lmi) {
        // Call for write lock...
        auto lock = this->getWriteLock();

        // First, try read...
        ptrdiff_t index = this->localizingMatrixIndex(lmi);
        if (index >= 0) {
            return dynamic_cast<class LocalizingMatrix&>(*matrices[momentMatrixIndices[index]]);
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
        return newLM;
    }





}