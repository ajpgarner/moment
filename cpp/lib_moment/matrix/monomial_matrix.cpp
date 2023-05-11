/**
 * monomial_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "monomial_matrix.h"
#include "operator_matrix.h"

#include "symbolic/symbol_table.h"
#include <stdexcept>

namespace Moment {

    namespace {
        using re_trip_t = Eigen::Triplet<sparse_real_elem_t::value_type>;
        using im_trip_t = Eigen::Triplet<sparse_complex_elem_t::value_type>;

        template<bool symmetric, bool complex>
        void do_create_dense_basis(const SymbolTable& symbols,
                                const SquareMatrix<SymbolExpression>& matrix,
                                Matrix::MatrixBasis::dense_real_storage_t& real,
                                Matrix::MatrixBasis::dense_complex_storage_t& im) {
            const int dimension = static_cast<int>(matrix.dimension);
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    const auto& elem = matrix[row_index][col_index];
                    assert(elem.id < symbols.size());
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        assert(re_id < real.size());
                        real[re_id](row_index, col_index) = elem.factor;

                        if constexpr(symmetric) {
                            if  (row_index != col_index) {
                                real[re_id](col_index, row_index) = elem.factor;
                            }
                        }
                    }

                    if constexpr (complex) {
                        if (im_id >= 0) {
                            assert(im_id < im.size());

                            im[im_id](row_index, col_index) =
                                    std::complex<double>(0.0, (elem.conjugated ? -1.0 : 1.0) * elem.factor);
                            if constexpr(symmetric) {
                                if (row_index != col_index) {
                                im[im_id](col_index, row_index) =
                                        std::complex<double>(0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor);
                                }
                            }
                        }
                    }
                }
            }
        }

        template<bool symmetric, bool complex>
        void do_create_sparse_frame(const SymbolTable& symbols,
                                    const SquareMatrix<SymbolExpression>& matrix,
                                    std::vector<std::vector<re_trip_t>>& real_frame,
                                    std::vector<std::vector<im_trip_t>>& im_frame) {

            const auto dimension = static_cast<int>(matrix.dimension);
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    const auto& elem = matrix[row_index][col_index];
                    assert(elem.id < symbols.size());
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        assert(re_id < real_frame.size());
                        real_frame[re_id].emplace_back(row_index, col_index, elem.factor);
                        if constexpr(symmetric) {
                            if (row_index != col_index) {
                                real_frame[re_id].emplace_back(col_index, row_index, elem.factor);
                            }
                        }
                    }

                    if constexpr(complex) {
                        if (im_id >= 0) {
                            assert(im_id < im_frame.size());
                            im_frame[im_id].emplace_back(row_index, col_index,
                                                         std::complex<double>(0, (elem.conjugated ? -1.0 : 1.0) *
                                                                                 elem.factor));
                            if constexpr(symmetric) {
                                if (row_index != col_index) {
                                    im_frame[im_id].emplace_back(col_index, row_index,
                                                                 std::complex<double>(0, (elem.conjugated ? 1.0 : -1.0) *
                                                                                         elem.factor));
                                }
                            }
                        }
                    }
                }
            }
        }

        /** Helper class, converts OSM -> Symbol matrix, registering new symbols */
        class OpSeqToSymbolConverter {
        private:
            const Context& context;
            SymbolTable& symbol_table;
            const OperatorMatrix::OpSeqMatrix& osm;
        public:
            const bool hermitian;
        public:
            OpSeqToSymbolConverter(const Context &context, SymbolTable &symbol_table,
                                   const OperatorMatrix::OpSeqMatrix &osm)
               : context{context}, symbol_table{symbol_table}, osm{osm}, hermitian{osm.is_hermitian()} { }


            std::unique_ptr<SquareMatrix<SymbolExpression>> operator()() {
                auto unique_sequences = hermitian ? identify_unique_sequences_hermitian()
                                                  : identify_unique_sequences_generic();

                symbol_table.merge_in(std::move(unique_sequences));

                return hermitian ? build_symbol_matrix_hermitian()
                                 : build_symbol_matrix_generic();
            }

        private:

            [[nodiscard]] std::vector<UniqueSequence> identify_unique_sequences_hermitian() const {
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
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            continue;
                        }

                        if (elem_hermitian) {
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

            [[nodiscard]] std::vector<UniqueSequence> identify_unique_sequences_generic() const {
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
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            continue;
                        }

                        if (elem_hermitian) {
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


            [[nodiscard]] std::unique_ptr<SquareMatrix<SymbolExpression>> build_symbol_matrix_hermitian() const {
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

            [[nodiscard]] std::unique_ptr<SquareMatrix<SymbolExpression>> build_symbol_matrix_generic() const {
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

                return std::make_unique<SquareMatrix<SymbolExpression>>(osm.dimension,
                        std::move(symbolic_representation));
            }
        };
    }

    MonomialMatrix::MonomialMatrix(SymbolTable& symbols, const Context& context,
                                   std::unique_ptr<SquareMatrix<SymbolExpression>> symbolMatrix,
                                   const bool is_hermitian)
        : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0},
            SymbolMatrix{*this}, sym_exp_matrix{std::move(symbolMatrix)}
        {
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to MonomialMatrix constructor was nullptr."};
            }

            // Find included symbols
            std::set<symbol_name_t> included_symbols;
            const size_t max_symbol_id = symbols.size();
            for (const auto& x : *sym_exp_matrix) {
                assert(x.id < max_symbol_id);
                included_symbols.emplace(x.id);
            }

            // Create symbol matrix properties
            this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols),
                                                                "Monomial Symbolic Matrix", is_hermitian);
    }

    MonomialMatrix::MonomialMatrix(SymbolTable &symbols, std::unique_ptr<OperatorMatrix> op_mat_ptr)
        : MonomialMatrix{symbols, op_mat_ptr->context,
                         OpSeqToSymbolConverter{op_mat_ptr->context, symbols, (*op_mat_ptr)()}(),
                         op_mat_ptr->is_hermitian()} {
        this->op_mat = std::move(op_mat_ptr);
        this->mat_prop = this->op_mat->replace_properties(std::move(this->mat_prop));
    }

    MonomialMatrix::~MonomialMatrix() noexcept = default;

    void MonomialMatrix::renumerate_bases(const SymbolTable &symbols) {
        for (auto& symbol : *this->sym_exp_matrix) {
            // Make conjugation status canonical:~
            if (symbol.conjugated) {
                const auto& ref_symbol = symbols[symbol.id];
                if (ref_symbol.is_hermitian()) {
                    symbol.conjugated = false;
                } else if (ref_symbol.is_antihermitian()) {
                    symbol.conjugated = false;
                    symbol.factor *= -1.0;
                }
            }
        }

        this->mat_prop->rebuild_keys(symbols);
    }

    std::pair<Matrix::MatrixBasis::dense_real_storage_t, Matrix::MatrixBasis::dense_complex_storage_t>
    MonomialMatrix::create_dense_basis() const {
        std::pair<Matrix::MatrixBasis::dense_real_storage_t, Matrix::MatrixBasis::dense_complex_storage_t> output;
        auto& real = output.first;
        auto& im = output.second;

        auto dim = static_cast<dense_real_elem_t::Index>(this->dimension);

        real.assign(this->symbol_table.Basis.RealSymbolCount(),
                    dense_real_elem_t::Zero(dim, dim));
        im.assign(this->symbol_table.Basis.ImaginarySymbolCount(),
                  dense_real_elem_t::Zero(dim, dim));

        const bool symmetric = this->SMP().IsHermitian();
        const bool complex = this->SMP().IsComplex();

        if (symmetric) {
            if (complex) {
                do_create_dense_basis<true, true>(this->Symbols, *this->sym_exp_matrix, real, im);
            } else {
                do_create_dense_basis<true, false>(this->Symbols, *this->sym_exp_matrix, real, im);
            }
        } else {
            if (complex) {
                do_create_dense_basis<false, true>(this->Symbols, *this->sym_exp_matrix, real, im);
            } else {
                do_create_dense_basis<false, false>(this->Symbols, *this->sym_exp_matrix, real, im);
            }
        }
        return output;
    }


    std::pair<Matrix::MatrixBasis::sparse_real_storage_t, Matrix::MatrixBasis::sparse_complex_storage_t>
    MonomialMatrix::create_sparse_basis() const {
        // Get matrix properties
        const auto dim = static_cast<sparse_real_elem_t::Index>(this->dimension);
        const bool symmetric = this->SMP().IsHermitian();
        const bool complex = this->SMP().IsComplex();

        // Prepare triplets
        std::vector<std::vector<re_trip_t>> real_frame(this->Symbols.Basis.RealSymbolCount());
        std::vector<std::vector<im_trip_t>> im_frame(this->Symbols.Basis.ImaginarySymbolCount());

        if (symmetric) {
            if (complex) {
                do_create_sparse_frame<true, true>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            } else {
                do_create_sparse_frame<true, false>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            }
        } else {
            if (complex) {
                do_create_sparse_frame<false, true>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            } else {
                do_create_sparse_frame<false, false>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            }
        }


        // Now, build sparse matrices
        std::pair<Matrix::MatrixBasis::sparse_real_storage_t, Matrix::MatrixBasis::sparse_complex_storage_t> output;
        auto& real = output.first;
        real.assign(real_frame.size(), sparse_real_elem_t(dim, dim));
        for (size_t re_index = 0; re_index < real_frame.size(); ++re_index) {
            real[re_index].setFromTriplets(real_frame[re_index].cbegin(), real_frame[re_index].cend());
        }

        if (complex) {
            auto &im = output.second;
            im.assign(im_frame.size(), sparse_complex_elem_t(dim, dim));
            for (size_t im_index = 0; im_index < im_frame.size(); ++im_index) {
                im[im_index].setFromTriplets(im_frame[im_index].cbegin(), im_frame[im_index].cend());
            }
        } else {
            // Null case: symbols are complex, but matrix is not.
            if (this->Symbols.Basis.ImaginarySymbolCount() > 0) {
                output.second.assign(this->Symbols.Basis.ImaginarySymbolCount(), sparse_complex_elem_t(dim, dim));
            }
        }

        // Return
        return output;
    }

}