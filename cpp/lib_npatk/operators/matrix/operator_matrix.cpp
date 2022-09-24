/**
 * operator_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include <limits>
#include "../operator_sequence_generator.h"
#include "../context.h"
#include "moment_matrix.h"
#include "operator_matrix.h"
#include "symbol_matrix_properties.h"


namespace NPATK {

    namespace {
        bool testForHermiticity(const SquareMatrix<OperatorSequence>& opMat) {
            for (size_t col = 0; col < opMat.dimension; ++col) {
                if (opMat[col][col] != opMat[col][col].conjugate()) {
                    return false;
                }
                for (size_t row = col+1; row < opMat.dimension; ++row) {
                    if (opMat[col][row] != opMat[row][col].conjugate()) {
                        return false;
                    }
                }
            }
            return true;
        }
    }

    OperatorMatrix::OperatorMatrix(const Context& context, SymbolTable& symbols,
                                   std::unique_ptr<SquareMatrix<OperatorSequence>> op_seq_mat)
       : context{context}, symbol_table{symbols}, Symbols{symbols}, SymbolMatrix{*this}, SequenceMatrix{*this},
         op_seq_matrix{std::move(op_seq_mat)}
       {
           assert(op_seq_matrix);

           // Get dimensions
           this->dimension = op_seq_matrix->dimension;

           // Test for Hermiticity
           this->is_hermitian = testForHermiticity(*this->op_seq_matrix);

           // Evaluate hashes (of entire matrix)
           std::vector<size_t> temporaryHashes{};
           temporaryHashes.reserve(this->dimension * this->dimension);
           for (const auto& seq : *(this->op_seq_matrix)) {
               temporaryHashes.emplace_back(context.hash(seq));
           }
           this->hash_matrix = std::make_unique<SquareMatrix<size_t>>(this->dimension, std::move(temporaryHashes));

           // Count unique strings in matrix (up to complex conjugation), and add to symbol table
           auto includedSymbols = this->integrateSymbols();

           // Calculate symbolic form of matrix
           this->sym_exp_matrix = this->buildSymbolMatrix();

           // Create symbol matrix properties
           this->sym_mat_prop = std::make_unique<SymbolMatrixProperties>(*this, this->symbol_table,
                                                                         std::move(includedSymbols));
       }


    OperatorMatrix::~OperatorMatrix() noexcept = default;


    std::set<symbol_name_t> OperatorMatrix::integrateSymbols() {
        auto unique_sequences = this->identifyUniqueSequences();
        return this->symbol_table.merge_in(std::move(unique_sequences));
    }


    std::vector<UniqueSequence> OperatorMatrix::identifyUniqueSequences() {
        if (this->is_hermitian) {
            return identifyUniqueSequencesHermitian();
        }
        return identifyUniqueSequencesGeneric();
    }

    std::vector<UniqueSequence>
    OperatorMatrix::identifyUniqueSequencesHermitian() {
        std::vector<UniqueSequence> build_unique;
        std::set<size_t> known_hashes;

        // First, manually insert zero and one
        build_unique.emplace_back(UniqueSequence::Zero(this->context));
        build_unique.emplace_back(UniqueSequence::Identity(this->context));
        known_hashes.emplace(0);
        known_hashes.emplace(1);


        // Now, look at elements and see if they are unique or not
        for (size_t row = 0; row < this->dimension; ++row) {
            for (size_t col = row; col < this->dimension; ++col) {
                const size_t hash = (*this->hash_matrix)[row][col];
                const size_t conj_hash = (*this->hash_matrix)[col][row]; // ONLY TRUE FOR HERMITIAN MATRIX
                const bool hermitian = (hash == conj_hash);

                // Don't add what is already known
                if (known_hashes.contains(hash) || (!hermitian && known_hashes.contains(conj_hash))) {
                    continue;
                }

                const auto& elem = this->SequenceMatrix[row][col];

                if (hermitian) {
                    build_unique.emplace_back(UniqueSequence{elem, hash});
                    known_hashes.emplace(hash);
                } else {
                    if (hash < conj_hash) {
                        build_unique.emplace_back(UniqueSequence{elem, hash, elem.conjugate(), conj_hash});
                    } else {
                        build_unique.emplace_back(UniqueSequence{elem.conjugate(), conj_hash, elem, hash});
                    }

                    known_hashes.emplace(hash);
                    known_hashes.emplace(conj_hash);
                }
            }
        }

        // NRVO?
        return build_unique;
    }


    std::vector<UniqueSequence>
    OperatorMatrix::identifyUniqueSequencesGeneric() {
        assert(false);
        throw;
    }


    std::unique_ptr<SquareMatrix<SymbolExpression>> OperatorMatrix::buildSymbolMatrix() {
        if (this->is_hermitian) {
            return buildSymbolMatrixHermitian();
        }
        return buildSymbolMatrixGeneric();
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    OperatorMatrix::buildSymbolMatrixHermitian() {
        std::vector<SymbolExpression> symbolic_representation(this->dimension * this->dimension);

        for (size_t row = 0; row < this->dimension; ++row) {
            for (size_t col = row; col < this->dimension; ++col) {
                size_t upper_index = (row * this->dimension) + col;
                size_t hash = (*this->hash_matrix)[row][col]; // temporaryHashes[upper_index];
                auto [symbol_id, conjugated] = this->symbol_table.hash_to_index(hash);
                if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                    throw;
                }
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

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    OperatorMatrix::buildSymbolMatrixGeneric() {
        assert(false);
        throw;
    }


}