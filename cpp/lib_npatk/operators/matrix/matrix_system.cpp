/**
 * matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_system.h"

#include "symbol_table.h"
#include "operator_matrix.h"
#include "moment_matrix.h"

#include "../context.h"
#include "../collins_gisin.h"
#include "../implicit_symbols.h"

#include <algorithm>
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
            this->cgForm = std::make_unique<CollinsGisinIndex>(*this, this->MaxRealSequenceLength());
            this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
        }
    }

    MatrixSystem::~MatrixSystem() noexcept = default;


    size_t MatrixSystem::MaxRealSequenceLength() const noexcept {
        // First, get length of the longest moment matrix:
        //  e.g. if [0] and [1] exists, size is 2, and MM of hierarchy level 1 guaranteed to exist.
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
        assert(this->hasMomentMatrix(level));
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
            if ((oldMPL < output.max_probability_length) || !this->cgForm || !this->implicitSymbols) {
                this->cgForm = std::make_unique<CollinsGisinIndex>(*this, this->MaxRealSequenceLength());
                this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
            }
        }

        return output;
    }

    const CollinsGisinIndex &MatrixSystem::CollinsGisin() const {
        if (!this->context->admits_cg_form()) {
            throw std::logic_error("CollinsGisin indexing not possible for this scenario.");
        }
        assert(this->cgForm);
        return *this->cgForm;
    }

    const ImplicitSymbols &MatrixSystem::ImplicitSymbolTable() const {
        if (!this->context->admits_cg_form()) {
            throw std::logic_error("Implicit symbols cannot be generated for this scenario.");
        }
        assert(this->implicitSymbols);
        return *this->implicitSymbols;
    }


}