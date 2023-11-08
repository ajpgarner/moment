/**
 * monomial_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * @see monomial_matrix_basis.cpp for basis function definitions.
 */
#include "monomial_matrix.h"
#include "operator_matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"
#include <stdexcept>

namespace Moment {

    namespace {
        /**
         * Helper class, converts OSM -> Symbol matrix, registering new symbols.
         * Note: this is the single-threaded implementation; see also /multithreading/matrix_generation_worker.h.
         */
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


            std::unique_ptr<SquareMatrix<Monomial>> operator()() {
                auto unique_sequences = hermitian ? identify_unique_sequences_hermitian()
                                                  : identify_unique_sequences_generic();

                symbol_table.merge_in(std::move(unique_sequences));

                return hermitian ? build_symbol_matrix_hermitian()
                                 : build_symbol_matrix_generic();
            }

        private:

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_hermitian() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);

                // Now, look at elements and see if they are unique or not
                auto lower_triangle = osm.LowerTriangle();
                auto iter = lower_triangle.begin();
                const auto iter_end = lower_triangle.end();
                while (iter != iter_end) {
                    // This is a bit of a hack to compensate for col-major storage, while preferring symbols to be
                    // numbered according to the top /row/ of moment matrices, if possible.
                    // Thus, we look at a col-major iterator over the lower triangle, which actually gives us the
                    // conjugates of what were generated; but we define what we find as the conjugate element.
                    const auto& conj_elem = *iter;
                    const auto elem = conj_elem.conjugate();

                    int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                    const bool elem_hermitian = (compare == 1);

                    const size_t hash = elem.hash();
                    const size_t conj_hash = conj_elem.hash();

                    // Don't add what is already known
                    if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                        ++iter;
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
                    ++iter;
                }
                // NRVO?
                return build_unique;
            }

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_generic() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);

                // Now, look at elements and see if they are unique or not
                for (const auto& elem : osm) {

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

                // NRVO?
                return build_unique;
            }


            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_hermitian() const {

                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);

                // Iterate over upper index
                auto upper_triangle_view = osm.UpperTriangle();
                auto iter = upper_triangle_view.begin();
                const auto iter_end = upper_triangle_view.end();
                while (iter != iter_end) {
                    const size_t row = iter.Row();
                    const size_t col = iter.Col();
                    const auto& elem = *iter;

                    const size_t hash = elem.hash();

                    const auto monomial_sign = to_scalar(elem.get_sign());

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << row << "," << col << "]"
                           << " was not found in symbol table, while parsing Hermitian matrix.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    symbolic_representation[iter.Offset()] = Monomial{unique_elem.Id(), monomial_sign, conjugated};

                    // Make Hermitian, if off-diagonal
                    if (!iter.diagonal()) {
                        size_t lower_offset = osm.index_to_offset_no_checks(std::array<size_t, 2>{col, row});
                        if (unique_elem.is_hermitian()) {
                            symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                             std::conj(monomial_sign), false};
                        } else {
                            symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                             std::conj(monomial_sign), !conjugated};
                        }
                    }
                    ++iter;

                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension, std::move(symbolic_representation));
            }

            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_generic() const {
                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);
                for (size_t offset = 0; offset < osm.ElementCount; ++offset) {
                    const auto& elem = osm[offset];

                    const auto monomial_sign = to_scalar(elem.get_sign());
                    const size_t hash = elem.hash();

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        auto index = osm.offset_to_index_no_checks(offset);
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << index[0] << "," << index[1] << "]"
                           << " was not found in symbol table.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    symbolic_representation[offset] = Monomial{unique_elem.Id(), monomial_sign, conjugated};
                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension,
                                                                std::move(symbolic_representation));
            }
        };
    }

    MonomialMatrix::MonomialMatrix(const Context& context, SymbolTable& symbols, const double zero_tolerance,
                                   std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                                   const bool constructed_as_hermitian)
        : SymbolicMatrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0},
          SymbolMatrix{*this}, sym_exp_matrix{std::move(symbolMatrix)}
        {
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to MonomialMatrix constructor was nullptr."};
            }

            // Count symbols
            this->MonomialMatrix::renumerate_bases(symbols, zero_tolerance);

            // Set  matrix properties
            this->description = "Monomial Symbolic Matrix";
            this->hermitian = constructed_as_hermitian;
    }

    MonomialMatrix::MonomialMatrix(SymbolTable &symbols, std::unique_ptr<OperatorMatrix> op_mat_ptr)
        : MonomialMatrix{op_mat_ptr->context, symbols, 1.0,
                         OpSeqToSymbolConverter{op_mat_ptr->context, symbols, (*op_mat_ptr)()}(),
                         op_mat_ptr->is_hermitian()} {
        assert(op_mat_ptr);
        this->op_mat = std::move(op_mat_ptr);

        // Count symbols
        this->MonomialMatrix::renumerate_bases(symbols, 1.0);

        // Set matrix properties
        this->op_mat->set_properties(*this);

    }

    MonomialMatrix::~MonomialMatrix() noexcept = default;

    void MonomialMatrix::renumerate_bases(const SymbolTable &symbols, double zero_tolerance) {
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
            // If zero, replace with canonical zero.
            if (approximately_zero(symbol.factor, zero_tolerance)) {
                symbol.id = 0;
                symbol.conjugated = false;
                symbol.factor = 0;
            }
        }

        this->identify_symbols_and_basis_indices();
    }

    void MonomialMatrix::identify_symbols_and_basis_indices() {
        // Find and canonicalize included symbols
        const size_t max_symbol_id = symbols.size();
        this->complex_coefficients = false;
        this->included_symbols.clear();
        for (auto& x : *sym_exp_matrix) {
            assert(x.id < max_symbol_id);
            this->included_symbols.emplace(x.id);
            if (!this->complex_coefficients && x.complex_factor()) { // <- first clause, avoid unnecessary tests
                this->complex_coefficients = true;
            }
        }

        // All included symbols:~
        this->real_basis_elements.clear();
        this->imaginary_basis_elements.clear();
        this->basis_key.clear();
        for (const auto symbol_id : this->included_symbols) {
            auto &symbol_info = this->symbols[symbol_id];
            auto [re_key, im_key] = symbol_info.basis_key();
            if (re_key >= 0) {
                this->real_basis_elements.emplace(re_key);
            }
            if (im_key >= 0) {
                this->imaginary_basis_elements.emplace(im_key);
            }
            this->basis_key.emplace_hint(this->basis_key.end(),
                                         std::make_pair(symbol_id, std::make_pair(re_key, im_key)));
        }

        this->complex_basis = !this->imaginary_basis_elements.empty();
    };

}