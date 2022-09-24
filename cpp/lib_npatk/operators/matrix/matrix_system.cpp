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
#include "../explicit_symbol.h"
#include "../implicit_symbols.h"
#include "../collins_gisin.h"

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

        // Create empty indexers
        if (this->context->admits_cg_form()) {
            this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, this->MaxRealSequenceLength());
            this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
        }
    }

    MatrixSystem::~MatrixSystem() noexcept = default;


    size_t MatrixSystem::MaxRealSequenceLength() const noexcept {
        // First, get length of the longest moment matrix:
        //  e.g. if [0] and [1] exists, size is 2, and MM of hierarchy Level 1 guaranteed to exist.
        ptrdiff_t hierarchy_level = static_cast<ptrdiff_t>(this->momentMatrixIndices.size()) - 1;
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of parties
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->context->Parties.size()));
    }

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

    const MomentMatrix &MatrixSystem::MomentMatrix(size_t level) const {
        if (!this->hasMomentMatrix(level)) {
            throw errors::missing_component("Moment matrix of Level " + std::to_string(level) + " not yet generated.");
        }
        return dynamic_cast<const class MomentMatrix&>(*matrices[momentMatrixIndices[level]]);
    }

    MomentMatrix& MatrixSystem::CreateMomentMatrix(size_t level) {
        // Call for write lock...
        std::unique_lock lock{this->rwMutex};

        // Old max probability length...
        auto oldMPL = this->MaxRealSequenceLength();

        // First, try read
        if (this->hasMomentMatrix(level)) {
            return dynamic_cast<class MomentMatrix&>(*matrices[momentMatrixIndices[level]]);
        }

        // Fill with null elements if some are missing
        if (this->momentMatrixIndices.size() < level+1) {
            this->momentMatrixIndices.resize(level+1, -1);
        }

        // Generate new moment matrix
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::make_unique<class MomentMatrix>(*this->context, *this->symbol_table, level));
        this->momentMatrixIndices[level] = matrixIndex;

        auto& output = dynamic_cast<class MomentMatrix&>(*this->matrices[matrixIndex]);

        // Check if new indices have become available, and if so, update...
        if (this->context->admits_cg_form()) {
            // Should we make explicit/implicit symbol table
            if ((oldMPL < output.max_probability_length) || !this->explicitSymbols || !this->implicitSymbols) {
                this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, this->MaxRealSequenceLength());
                this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
            }

            // Can/should we make C-G tensor?
            if (!this->collinsGisin && (output.max_probability_length >= this->context->Parties.size())) {
                this->collinsGisin = std::make_unique<NPATK::CollinsGisin>(*this);
            }
        }

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

    LocalizingMatrix& MatrixSystem::CreateLocalizingMatrix(LocalizingMatrixIndex lmi) {
        // Call for write lock...
        std::unique_lock lock{this->rwMutex};

        // First, try read...
        ptrdiff_t index = this->localizingMatrixIndex(lmi);
        if (index >= 0) {
            return dynamic_cast<class LocalizingMatrix&>(*matrices[momentMatrixIndices[index]]);
        }

        // Otherwise,generate new localizing matrix, and insert index
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::make_unique<class LocalizingMatrix>(*this->context, *this->symbol_table, lmi));
        this->localizingMatrixIndices.emplace(std::make_pair(lmi, matrixIndex));

        // Return (reference to) matrix just added
        return dynamic_cast<class LocalizingMatrix&>(*this->matrices.back());

    }

    const ExplicitSymbolIndex& MatrixSystem::ExplicitSymbolTable() const {
        if (!this->context->admits_cg_form()) {
            throw errors::missing_component("ExplicitSymbolTable indexing not possible for this scenario.");
        }
        if (!this->explicitSymbols) {
            throw errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    const ImplicitSymbols& MatrixSystem::ImplicitSymbolTable() const {
        if (!this->context->admits_cg_form()) {
            throw errors::missing_component("Implicit symbols cannot be generated for this scenario.");
        }
        if (!this->implicitSymbols) {
            throw errors::missing_component("ImplicitSymbolTable has not yet been generated.");
        }
        return *this->implicitSymbols;
    }

    const CollinsGisin& MatrixSystem::CollinsGisin() const {
        if (!this->context->admits_cg_form()) {
            throw errors::missing_component("Implicit symbols cannot be generated for this scenario.");
        }
        if (!this->collinsGisin) {
            throw errors::missing_component(std::string("Collins-Gisin tensor has not yet been generated. ")
                                     + "Perhaps a large enough moment matrix has not yet been created.");
        }
        return *this->collinsGisin;
    }



}