/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "operator_sequence_generator.h"

#include "matrix_properties.h"

#include <limits>
#include <stdexcept>
#include <sstream>

namespace Moment {
    namespace {


        std::unique_ptr<SquareMatrix<SymbolExpression>>
        registerSymbolsAndBuildMatrix(const Context &context, SymbolTable &symbol_table,
                                      const OperatorMatrix::OpSeqMatrix &osm);

        std::vector<UniqueSequence> identifyUniqueSequencesHermitian(const Context& context,
                                                                     const OperatorMatrix::OpSeqMatrix& osm);

        std::vector<UniqueSequence> identifyUniqueSequencesGeneric(const Context& context,
                                                                   const OperatorMatrix::OpSeqMatrix& osm);

        std::unique_ptr<SquareMatrix<SymbolExpression>>
        buildSymbolMatrixHermitian(SymbolTable& symbol_table, const OperatorMatrix::OpSeqMatrix& osm);

        std::unique_ptr<SquareMatrix<SymbolExpression>>
        buildSymbolMatrixGeneric(SymbolTable& symbol_table, const OperatorMatrix::OpSeqMatrix& osm);

        std::unique_ptr<SquareMatrix<SymbolExpression>>
        registerSymbolsAndBuildMatrix(const Context& context, SymbolTable& symbol_table,
                                      const OperatorMatrix::OpSeqMatrix& osm) {
            const bool hermitian = osm.is_hermitian();
            auto unique_sequences = hermitian ? identifyUniqueSequencesHermitian(context, osm)
                                              : identifyUniqueSequencesGeneric(context, osm);

            symbol_table.merge_in(std::move(unique_sequences));

            return hermitian ? buildSymbolMatrixHermitian(symbol_table, osm)
                             : buildSymbolMatrixGeneric(symbol_table, osm);
        }
        
        std::vector<UniqueSequence>
        identifyUniqueSequencesHermitian(const Context& context, const OperatorMatrix::OpSeqMatrix& osm) {
            std::vector<UniqueSequence> build_unique;
            std::set<size_t> known_hashes;

            // First, always manually insert zero and one
            build_unique.emplace_back(UniqueSequence::Zero(context));
            build_unique.emplace_back(UniqueSequence::Identity(context));
            known_hashes.emplace(0);
            known_hashes.emplace(1);

            // Now, look at elements and see if they are unique or not
            for (size_t row = 0; row < osm.dimension; ++row) {
                for (size_t col = row; col < osm.dimension; ++col) {
                    const auto& elem = osm[row][col];
                    const auto conj_elem = elem.conjugate();
                    int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                    const bool hermitian = (compare == 1);

                    const size_t hash = elem.hash();
                    const size_t conj_hash = conj_elem.hash();

                    // Don't add what is already known
                    if (known_hashes.contains(hash) || (!hermitian && known_hashes.contains(conj_hash))) {
                        continue;
                    }

                    if (hermitian) {
                        build_unique.emplace_back(elem);
                        known_hashes.emplace(hash);
                    } else {
                        if (hash < conj_hash) {
                            build_unique.emplace_back(elem, conj_elem);
                        } else {
                            build_unique.emplace_back(conj_elem, elem);
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
        identifyUniqueSequencesGeneric(const Context& context, const OperatorMatrix::OpSeqMatrix& osm) {
            std::vector<UniqueSequence> build_unique;
            std::set<size_t> known_hashes;

            // First, always manually insert zero and one
            build_unique.emplace_back(UniqueSequence::Zero(context));
            build_unique.emplace_back(UniqueSequence::Identity(context));
            known_hashes.emplace(0);
            known_hashes.emplace(1);

            // Now, look at elements and see if they are unique or not
            for (size_t row = 0; row < osm.dimension; ++row) {
                for (size_t col = 0; col < osm.dimension; ++col) {
                    const auto& elem = osm[row][col];
                    const auto conj_elem = elem.conjugate();
                    int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                    const bool hermitian = (compare == 1);

                    const size_t hash = elem.hash();
                    const size_t conj_hash = conj_elem.hash();

                    // Don't add what is already known
                    if (known_hashes.contains(hash) || (!hermitian && known_hashes.contains(conj_hash))) {
                        continue;
                    }

                    if (hermitian) {
                        build_unique.emplace_back(elem);
                        known_hashes.emplace(hash);
                    } else {
                        if (hash < conj_hash) {
                            build_unique.emplace_back(elem, conj_elem);
                        } else {
                            build_unique.emplace_back(conj_elem, elem);
                        }

                        known_hashes.emplace(hash);
                        known_hashes.emplace(conj_hash);
                    }
                }
            }

            // NRVO?
            return build_unique;
        }


        std::unique_ptr<SquareMatrix<SymbolExpression>> 
        buildSymbolMatrixHermitian(SymbolTable& symbol_table, const OperatorMatrix::OpSeqMatrix& osm) {
            std::vector<SymbolExpression> symbolic_representation(osm.dimension * osm.dimension);

            for (size_t row = 0; row < osm.dimension; ++row) {
                for (size_t col = row; col < osm.dimension; ++col) {
                    const size_t upper_index = (row * osm.dimension) + col;
                    const auto& elem = osm[row][col];
                    const size_t hash = elem.hash();
                    const bool negated = elem.negated();

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << osm[row][col] << "\" at index [" << row << "," << col << "]"
                           << " was not found in symbol table, while parsing Hermitian matrix.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    symbolic_representation[upper_index] = SymbolExpression{unique_elem.Id(), negated, conjugated};

                    // Make Hermitian, if off-diagonal
                    if (col > row) {
                        size_t lower_index = (col * osm.dimension) + row;
                        if (unique_elem.is_hermitian()) {
                            symbolic_representation[lower_index] = SymbolExpression{unique_elem.Id(),
                                                                                    negated, false};
                        } else {
                            symbolic_representation[lower_index] = SymbolExpression{unique_elem.Id(),
                                                                                    negated, !conjugated};
                        }
                    }
                }
            }

            return std::make_unique<SquareMatrix<SymbolExpression>>(osm.dimension, std::move(symbolic_representation));
        }

        std::unique_ptr<SquareMatrix<SymbolExpression>> 
        buildSymbolMatrixGeneric(SymbolTable& symbol_table, const OperatorMatrix::OpSeqMatrix& osm) {
            std::vector<SymbolExpression> symbolic_representation(osm.dimension * osm.dimension);
            for (size_t row = 0; row < osm.dimension; ++row) {
                for (size_t col = 0; col < osm.dimension; ++col) {
                    const size_t index = (row * osm.dimension) + col;
                    const auto& elem = osm[row][col];
                    const bool negated = elem.negated();
                    const size_t hash = elem.hash();

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << osm[row][col] << "\" at index [" << row << "," << col << "]"
                           << " was not found in symbol table.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    symbolic_representation[index] = SymbolExpression{unique_elem.Id(), negated, conjugated};
                }
            }

            return std::make_unique<SquareMatrix<SymbolExpression>>(osm.dimension, std::move(symbolic_representation));
        }
    }

    OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)) {
        this->calculate_hermicity();
    }

    void OperatorMatrix::OpSeqMatrix::calculate_hermicity() {
        for (size_t row = 0; row < this->dimension; ++row) {
            if ((*this)[row][row] != (*this)[row][row].conjugate()) {
                this->hermitian = false;
                this->nonh_i = static_cast<ptrdiff_t>(row);
                this->nonh_j = static_cast<ptrdiff_t>(row);
                return;
            }
            for (size_t col = row+1; col < this->dimension; ++col) {
                const auto& upper = (*this)[row][col];
                const auto& lower = (*this)[col][row];
                const auto lower_conj = lower.conjugate();
                if (upper != lower_conj) {
                    this->hermitian = false;
                    this->nonh_i = static_cast<ptrdiff_t>(row);
                    this->nonh_j = static_cast<ptrdiff_t>(col);
                    return;
                }
            }
        }
        this->hermitian = true;
        this->nonh_i = this->nonh_j = -1;
    }

    OperatorMatrix::OperatorMatrix(const Context& context, SymbolTable& symbols,
                                   std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
       : MonomialMatrix{context, symbols, registerSymbolsAndBuildMatrix(context, symbols, *op_seq_mat)},
         SequenceMatrix{*this}, op_seq_matrix{std::move(op_seq_mat)}
       {
           assert(op_seq_matrix);

           // Test for Hermiticity
           this->is_hermitian = op_seq_matrix->is_hermitian();

       }

    OperatorMatrix::~OperatorMatrix() noexcept = default;
}