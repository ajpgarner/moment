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

#include <limits>


namespace NPATK {
    namespace {
        constexpr size_t getMaxProbLen(const Context& context, size_t hierarchy_level) {
            return std::min(hierarchy_level*2, context.Parties.size());
        }
    }

    MomentMatrix::MomentMatrix(std::shared_ptr<Context> theContextPtr, size_t level)
        : contextPtr{std::move(theContextPtr)}, context{*contextPtr}, hierarchy_level{level},
            max_probability_length{getMaxProbLen(context, hierarchy_level)},
            UniqueSequences{*this}, SymbolMatrix{*this} {

        // Prepare generator of symbols
        OperatorSequenceGenerator colGen{context, hierarchy_level};
        OperatorSequenceGenerator rowGen{colGen.conjugate()};

        // Build matrix...
        this->matrix_dimension = colGen.size();
        assert(this->matrix_dimension == rowGen.size());
        std::vector<OperatorSequence> matrix_data;
        matrix_data.reserve(this->matrix_dimension * this->matrix_dimension);

        std::vector<size_t> temporaryHashes{};
        temporaryHashes.reserve(this->matrix_dimension * this->matrix_dimension);
        for (const auto& rowSeq : rowGen) {
            for (const auto& colSeq : colGen) {
                matrix_data.emplace_back(rowSeq * colSeq);
                temporaryHashes.emplace_back(context.hash(matrix_data.back()));
            }
        }
        this->op_seq_matrix = std::make_unique<SquareMatrix<OperatorSequence>>(matrix_dimension,
                                                                               std::move(matrix_data));

        // Count unique strings in matrix (up to complex conjugation)
        this->identifyUniqueSequences(temporaryHashes);

        // Convert matrix to symbolic form...
        this->sym_exp_matrix = this->buildSymbolMatrix(temporaryHashes);

        // Create index matrix
        this->imp = IndexMatrixProperties{*this};
    }

    MomentMatrix::MomentMatrix(MomentMatrix &&src) noexcept:
            contextPtr{std::move(src.contextPtr)}, context{*contextPtr},
            hierarchy_level{src.hierarchy_level}, max_probability_length{src.max_probability_length},
            UniqueSequences{*this}, SymbolMatrix{*this},
            matrix_dimension{src.matrix_dimension},
            op_seq_matrix{std::move(src.op_seq_matrix)},
            sym_exp_matrix{std::move(src.sym_exp_matrix)},
            unique_sequences{std::move(src.unique_sequences)},
            imp{std::move(src.imp)},
            fwd_hash_table{std::move(src.fwd_hash_table)},
            conj_hash_table{std::move(src.conj_hash_table)},
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

    const MomentMatrix::UniqueSequence *
    MomentMatrix::UniqueSequenceRange::where(const OperatorSequence &seq) const noexcept {
        size_t hash = this->matrix.context.hash(seq);

        auto [id, conj] = this->matrix.hashToElement(hash);
        if (id == std::numeric_limits<size_t>::max()) {
            return nullptr;
        }

        assert(id < this->matrix.unique_sequences.size());
        return &this->matrix.unique_sequences[id];
    }

    SymbolExpression MomentMatrix::UniqueSequenceRange::to_symbol(const OperatorSequence &seq) const noexcept {
        size_t hash = this->matrix.context.hash(seq);
        auto [id, conj] = this->matrix.hashToElement(hash);
        if (id == std::numeric_limits<size_t>::max()) {
            return SymbolExpression{0};
        }

        return SymbolExpression{this->matrix.unique_sequences[id].id, conj};
    }


    void MomentMatrix::identifyUniqueSequences(const std::vector<size_t> &temporaryHashes) {
        std::map<size_t, UniqueSequence> build_unique;
        std::map<size_t, size_t> conj_alias;

        // First, manually insert zero and one
        build_unique.emplace(0, UniqueSequence::Zero(this->context));
        build_unique.emplace(1, UniqueSequence::Identity(this->context));



        for (size_t row = 0; row < matrix_dimension; ++row) {
            for (size_t col = row; col < matrix_dimension; ++col) {
                const auto& elem = (*this)[row][col]; //(row * matrix_dimension) + col];
                const auto& conj_elem = (*this)[col][row];
                bool hermitian = (elem == conj_elem);
                size_t hash = temporaryHashes[(row * matrix_dimension) + col];
                size_t conj_hash = temporaryHashes[(col * matrix_dimension) + row];

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

        // Flatten
        unique_sequences.reserve(build_unique.size());
        size_t count = 0; // 0 and 1 already in list...!

        for (auto& [hash, elem] : build_unique) {
            bool hermitian = elem.hermitian;
            elem.id = static_cast<symbol_name_t>(count);
            unique_sequences.emplace_back(std::move(elem));
            fwd_hash_table.emplace_hint(fwd_hash_table.end(), hash, count);
            if (!hermitian) {
                conj_hash_table.emplace(unique_sequences[count].conj_hash, count);
            }
            ++count;
        }
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    MomentMatrix::buildSymbolMatrix(const std::vector<size_t> &temporaryHashes) {
        std::vector<SymbolExpression> symbolic_representation(matrix_dimension * matrix_dimension);

        for (size_t row = 0; row < matrix_dimension; ++row) {
            for (size_t col = row; col < matrix_dimension; ++col) {
                size_t upper_index = (row * matrix_dimension) + col;
                size_t hash = temporaryHashes[upper_index];
                auto [symbol_id, conjugated] = hashToElement(hash);
                assert(symbol_id != std::numeric_limits<size_t>::max());
                const auto& unique_elem = unique_sequences[symbol_id];

                symbolic_representation[upper_index] = SymbolExpression{unique_elem.id, conjugated};

                // Make Hermitian, if off-diagonal
                if (col > row) {
                    size_t lower_index = (col * matrix_dimension) + row;
                    if (unique_elem.hermitian) {
                        symbolic_representation[lower_index] = SymbolExpression{unique_elem.id, false};
                    } else {
                        symbolic_representation[lower_index] = SymbolExpression{unique_elem.id, !conjugated};
                    }
                }
            }
        }

        return std::make_unique<SquareMatrix<SymbolExpression>>(matrix_dimension, std::move(symbolic_representation));
    }

    std::pair<size_t, bool> MomentMatrix::hashToElement(size_t hash) const noexcept {
        auto fwd_hash_iter = this->fwd_hash_table.find(hash);
        if (fwd_hash_iter != this->fwd_hash_table.end()) {
            return {fwd_hash_iter->second, false};
        }

        auto conj_hash_iter = this->conj_hash_table.find(hash);
        if (conj_hash_iter != this->conj_hash_table.end()) {
            return {conj_hash_iter->second, true};
        }

        return {std::numeric_limits<size_t>::max(), false};
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