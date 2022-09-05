/**
 * npa_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "moment_matrix.h"
#include "context.h"
#include "collins_gisin.h"
#include "implicit_symbols.h"
#include "operator_sequence_generator.h"
#include "operator_matrix.h"

#include <limits>


namespace NPATK {
    namespace {
        constexpr size_t getMaxProbLen(const Context& context, size_t hierarchy_level) {
            return std::min(hierarchy_level*2, context.Parties.size());
        }
    }

    MomentMatrix::MomentMatrix(const Context& context, SymbolTable& symbols, size_t level)
        : OperatorMatrix{context, symbols}, hierarchy_level{level},
          max_probability_length{getMaxProbLen(context, hierarchy_level)} {

        // Prepare generator of symbols
        OperatorSequenceGenerator colGen{context, hierarchy_level};
        OperatorSequenceGenerator rowGen{colGen.conjugate()};

        // Build matrix...
        this->dimension = colGen.size();
        assert(this->dimension == rowGen.size());
        std::vector<OperatorSequence> matrix_data;
        matrix_data.reserve(this->dimension * this->dimension);

        std::vector<size_t> temporaryHashes{};
        temporaryHashes.reserve(this->dimension * this->dimension);
        for (const auto& rowSeq : rowGen) {
            for (const auto& colSeq : colGen) {
                matrix_data.emplace_back(rowSeq * colSeq);
                temporaryHashes.emplace_back(context.hash(matrix_data.back()));
            }
        }

        this->op_seq_matrix = std::make_unique<SquareMatrix<OperatorSequence>>(this->dimension,
                                                                               std::move(matrix_data));

        // Count unique strings in matrix (up to complex conjugation), and add to symbol table
        auto hash_to_unique_seq = this->identifyUniqueSequences(temporaryHashes);
        auto includedSymbols = this->symbol_table.merge_in(std::move(hash_to_unique_seq));

        // Calculate symbolic form of matrix...
        this->sym_exp_matrix = this->buildSymbolMatrix(temporaryHashes);

        // Create symbol matrix properties
        this->sym_mat_prop = std::make_unique<SymbolMatrixProperties>(*this, this->symbol_table,
                                                                      std::move(includedSymbols));
    }

    MomentMatrix::MomentMatrix(MomentMatrix &&src) noexcept :
            OperatorMatrix{static_cast<OperatorMatrix&&>(src)},
            hierarchy_level{src.hierarchy_level}, max_probability_length{src.max_probability_length},
            cgForm{std::move(src.cgForm)},
            implicitSymbols{std::move(src.implicitSymbols)} {

        // Do we have CG form already?
        if (this->cgForm) {
            this->cgFormExists.test_and_set();
            this->cgFormConstructFlag.test_and_set();
        } else {
            this->cgFormExists.clear();
            this->cgFormConstructFlag.clear();
        }

        // Do we have implicit symbol already?
        if (this->implicitSymbols) {
            this->impSymExists.test_and_set();
            this->impSymConstructFlag.test_and_set();
        } else {
            this->impSymExists.clear();
            this->impSymConstructFlag.clear();
        }
    }

    MomentMatrix::~MomentMatrix() = default;

    std::map<size_t, UniqueSequence> MomentMatrix::identifyUniqueSequences(const std::vector<size_t> &temporaryHashes) {
        std::map<size_t, UniqueSequence> build_unique;
        std::map<size_t, size_t> conj_alias;

        // First, manually insert zero and one
        build_unique.emplace(0, UniqueSequence::Zero(this->context));
        build_unique.emplace(1, UniqueSequence::Identity(this->context));

        // Now, look at elements and see if they are unique or not
        for (size_t row = 0; row < this->dimension; ++row) {
            for (size_t col = row; col < this->dimension; ++col) {
                const auto& elem = this->SequenceMatrix[row][col];
                const auto& conj_elem = this->SequenceMatrix[col][row];
                bool hermitian = (elem == conj_elem);
                size_t hash = temporaryHashes[(row * this->dimension) + col];
                size_t conj_hash = temporaryHashes[(col * this->dimension) + row];

                if (hermitian) {
                    // Does exist?
                    if (!build_unique.contains(hash) && !conj_alias.contains(hash)) {
                        build_unique.emplace(hash, UniqueSequence{elem, hash});
                    }
                } else {
                    // Does exist?
                    if (!build_unique.contains(hash) && !conj_alias.contains(hash)) {
                        build_unique.emplace(hash, UniqueSequence{elem, hash, conj_elem, conj_hash});
                        conj_alias.emplace(conj_hash, hash);
                    }
                }
            }
        }

        // NRVO?
        return build_unique;
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    MomentMatrix::buildSymbolMatrix(const std::vector<size_t> &temporaryHashes) {
        std::vector<SymbolExpression> symbolic_representation(this->dimension * this->dimension);

        for (size_t row = 0; row < this->dimension; ++row) {
            for (size_t col = row; col < this->dimension; ++col) {
                size_t upper_index = (row * this->dimension) + col;
                size_t hash = temporaryHashes[upper_index];
                auto [symbol_id, conjugated] = this->symbol_table.hash_to_index(hash);
                assert(symbol_id != std::numeric_limits<size_t>::max());
                const auto& unique_elem = this->symbol_table[symbol_id];

                symbolic_representation[upper_index] = SymbolExpression{unique_elem.Id(), conjugated};

                // Make Hermitian, if off-diagonal
                if (col > row) {
                    size_t lower_index = (col * this->dimension) + row;
                    if (unique_elem.is_hermitian()) {
                        symbolic_representation[lower_index] = SymbolExpression{unique_elem.Id(), false};
                    } else {
                        symbolic_representation[lower_index] = SymbolExpression{unique_elem.Id(), !conjugated};
                    }
                }
            }
        }

        return std::make_unique<SquareMatrix<SymbolExpression>>(this->dimension, std::move(symbolic_representation));
    }


    const CollinsGisinForm& MomentMatrix::CollinsGisin() const {
        // First thread to enter function constructs object
        if (!this->cgFormConstructFlag.test_and_set(std::memory_order_acquire)) {
            auto newCGform = std::make_unique<CollinsGisinForm>(*this, max_probability_length);
            const_cast<MomentMatrix*>(this)->cgForm.swap(newCGform);
            this->cgFormExists.test_and_set(std::memory_order_release);
            this->cgFormExists.notify_all();
            return *this->cgForm;
        } else {
            // Other threads spin until construction is done, if necessary
            while (!this->cgFormExists.test(std::memory_order_acquire)) {
                this->cgFormExists.wait(false);
            }
            return *this->cgForm;
        }
    }

    const ImplicitSymbols& MomentMatrix::ImplicitSymbolTable() const {
        // First thread to enter function constructs object
        if (!this->impSymConstructFlag.test_and_set(std::memory_order_acquire)) {
            auto newImpSym = std::make_unique<ImplicitSymbols>(*this);
            const_cast<MomentMatrix*>(this)->implicitSymbols.swap(newImpSym);
            this->impSymExists.test_and_set(std::memory_order_release);
            this->impSymExists.notify_all();
            return *this->implicitSymbols;
        } else {
            // Other threads spin until construction is done, if necessary
            while (!this->impSymExists.test(std::memory_order_acquire)) {
                this->impSymExists.wait(false);
            }
            return *this->implicitSymbols;
        }
    }


}