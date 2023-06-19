/**
 * imported_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "imported_matrix_system.h"
#include "imported_context.h"

#include "matrix/monomial_matrix.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/dynamic_bitset.h"

#include <cassert>
#include <cmath>
#include <sstream>

namespace Moment::Imported {

    namespace {
        void checkIMSymmetric(const SquareMatrix<Monomial>& input,
                            DynamicBitset<uint64_t>& can_be_real, DynamicBitset<uint64_t>& can_be_imaginary) {
            for (size_t i = 0; i < input.dimension; ++i) {
                const auto& diagonal = input[i][i];

                for (size_t j = i+1; j < input.dimension; ++j) {
                    auto& upper = input[i][j];
                    auto& lower = input[j][i];
                    if ((upper.id != lower.id) || (abs(upper.factor) != abs(lower.factor))) {
                        std::stringstream errSS;
                        errSS << "In symmetric matrix import, element [" << i << ", " << j << "] = "
                              << upper.as_string() << " does not match element [" << j << ", " << i << "] = " << lower;

                        throw errors::bad_import_matrix{errSS.str()};
                    }
                    const bool conj_eq = upper.conjugated == lower.conjugated;
                    const bool neg_eq = upper.factor == lower.factor;

                    if (conj_eq) {
                        if (!neg_eq) { // a = -a -> a is zero
                            can_be_real.unset(upper.id);
                            can_be_imaginary.unset(upper.id);
                        } // else a = a, no constraints
                    } else if (neg_eq) { // a = a* -> a is real
                        can_be_imaginary.unset(upper.id);
                    } else { // a = -a* -> a is imaginary
                        can_be_real.unset(upper.id);
                    }
                }
            }
        }

        void checkIMHermitian(const SquareMatrix<Monomial>& input, bool can_be_complex,
                              DynamicBitset<uint64_t>& can_be_real, DynamicBitset<uint64_t>& can_be_imaginary) {

            for (size_t i = 0; i < input.dimension; ++i) {
                const auto &diagonal = input[i][i];
                // Diagonal elements are always real;
                can_be_imaginary.unset(diagonal.id);

                for (size_t j = i + 1; j < input.dimension; ++j) {
                    auto &upper = input[i][j];
                    auto &lower = input[j][i];

                    if ((upper.id != lower.id) || (abs(upper.factor) != abs(lower.factor))) {
                        std::stringstream errSS;
                        errSS << "In hermitian matrix import, element [" << i << ", " << j << "] = "
                              << upper.as_string() << " does not match element [" << j << ", " << i << "] = " << lower;

                        throw errors::bad_import_matrix{errSS.str()};
                    }
                    const bool conj_eq = upper.conjugated != lower.conjugated; // <- account for Hermiticity
                    const bool neg_eq = upper.factor == lower.factor;

                    if (!conj_eq) {
                        if (neg_eq) { // a* = a -> a = -a* -> a is real
                            can_be_imaginary.unset(upper.id);
                        } else { // else a* = -a -> a = -a* -> a is imaginary
                            can_be_real.unset(upper.id);
                        }
                    } else if (!neg_eq) { // a = -a -> a is zero
                        can_be_real.unset(upper.id);
                        can_be_imaginary.unset(upper.id);
                    } // else, a* = a*, no constraints

                    // In real-only context, no symbol mentioned can be imaginary
                    if (!can_be_complex) {
                        can_be_imaginary.unset(upper.id);
                    }
                }
            }
        }

    }

    ImportedMatrixSystem::ImportedMatrixSystem(const bool purely_real)
        : MatrixSystem(std::make_unique<ImportedContext>(purely_real)),
          importedContext{dynamic_cast<ImportedContext&>(this->Context())} {

    }

    std::unique_ptr<class Matrix>
    ImportedMatrixSystem::createNewMomentMatrix(size_t level, Multithreading::MultiThreadPolicy mt_policy) {
        throw std::runtime_error{"Operator matrices cannot be procedurally generated in imported context."};
    }

    std::unique_ptr<class Matrix>
    ImportedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi,
                                                    Multithreading::MultiThreadPolicy mt_policy) {
        throw std::runtime_error{"Operator matrices cannot be procedurally generated in imported context."};
    }

    size_t ImportedMatrixSystem::import_matrix(std::unique_ptr<SquareMatrix<Monomial>> input,
                                               const bool is_complex, const bool is_hermitian) {
        assert(input);

        // Complain if context is real, but matrix is not.
        if (this->importedContext.real_only() && is_complex) {
            throw errors::bad_import_matrix{"Cannot import complex matrix into purely real context."};
        }

        // Real context only defines real symbols, and real import only provides real symbols
        const bool can_be_complex = !this->importedContext.real_only() && is_complex;

        // Parse for largest symbol identity
        const size_t initial_symbol_size = this->Symbols().size();
        size_t new_max_symbol_id = (initial_symbol_size > 0) ? (initial_symbol_size - 1) : 0;
        for (const auto& symbol_expression : *input) {
            if (symbol_expression.id > new_max_symbol_id) {
                new_max_symbol_id = symbol_expression.id;
            }
        }

        // Flag whether a symbol can be real
        DynamicBitset can_be_real{new_max_symbol_id+1, true};

        // Flag whether a symbol can be imaginary
        DynamicBitset can_be_imaginary{new_max_symbol_id+1, !this->importedContext.real_only()};

        // Check if import type implies real or imaginary parts of mentioned symbols should be zero
        if (is_hermitian) {
            if (is_complex) {
                checkIMHermitian(*input, can_be_complex, can_be_real, can_be_imaginary);
            } else {
                checkIMSymmetric(*input, can_be_real, can_be_imaginary);
            }
        }

        // If importing real matrix into complex system, all imported reference symbols must be real.
        if (!is_complex) {
            if (!this->importedContext.real_only()) {
                for (auto &symbol: *input) {
                    can_be_imaginary.unset(symbol.id);
                }
            }
        }

        // Now, prepare to import
        auto write_lock = this->get_write_lock();

        // Do merge, renumbering bases as appropriate, and complaining if a symbol becomes zero.
        bool changed_symbols = false;
        try {
            changed_symbols = this->Symbols().merge_in(can_be_real, can_be_imaginary);
        } catch (const Moment::errors::zero_symbol& zse) {
            throw errors::bad_import_matrix{zse.what()}; // Likely exception: importing alias for '0'
        }

        // Reconstruct bases for all existing matrices, if any symbol changed from real to complex or vice versa.
        if (changed_symbols) {
            for (size_t index = 0; index < this->size(); ++index) {
                auto &old_mat = this->get(index);
                old_mat.renumerate_bases(this->Symbols(), this->polynomial_factory().zero_tolerance);
            }
        }

        // Construct new symbolic matrix
        return this->push_back(
            std::make_unique<MonomialMatrix>(this->Context(), this->Symbols(),
                                             this->polynomial_factory().zero_tolerance,
                                             std::move(input), is_hermitian)
        );
    }
}