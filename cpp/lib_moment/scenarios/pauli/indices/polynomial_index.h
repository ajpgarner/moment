/**
 * polynomial_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "nearest_neighbour_index.h"
#include "monomial_index.h"

#include "matrix_system/matrix_indices.h"
#include "matrix_system/indices/polynomial_localizing_matrix_index.h"
#include "matrix_system/index_storage/polynomial_index_storage.h"

#include "multithreading/maintains_mutex.h"

#include <iosfwd>
#include <string>

namespace Moment {
    class MatrixSystem;
    class PolynomialMatrix;

    namespace Pauli {
        /**
         * Index of of NPA Hierarchy level, nearest neighbour info, and a Polynomial.
          */
        template<typename constituent_index_t>
        struct PauliPolynomialIndex
                : public PolynomialIndexBase<NearestNeighbourIndex, constituent_index_t> {
        public:
            using OSGIndex = NearestNeighbourIndex;
            using ComponentIndex = constituent_index_t;

            /** Construct Pauli-scenario polynomial localizing matrix by initiating nearest neighbour info.. */
            PauliPolynomialIndex(const size_t level, const size_t neighbours, class Polynomial poly) noexcept
                : PolynomialIndexBase<NearestNeighbourIndex, constituent_index_t>{
                        NearestNeighbourIndex{level, neighbours}, std::move(poly)} { }

            /** Downcast, ignoring nearest neighbour info. */
            explicit operator PolynomialLocalizingMatrixIndex() const {
                return {this->Level.moment_matrix_level, this->Polynomial};
            }
        };

        /**
         * Index of a polynomial localizing matrix in the Pauli scenario.
         */
        struct PolynomialLocalizingMatrixIndex : public PauliPolynomialIndex<Pauli::LocalizingMatrixIndex> {
        public:
            PolynomialLocalizingMatrixIndex(const size_t level, const size_t neighbours, class Polynomial poly) noexcept
                : PauliPolynomialIndex<Pauli::LocalizingMatrixIndex>{level, neighbours, std::move(poly)} { }

            PolynomialLocalizingMatrixIndex(const NearestNeighbourIndex& nn_index, class Polynomial poly) noexcept
                : PauliPolynomialIndex<Pauli::LocalizingMatrixIndex>{nn_index.moment_matrix_level, nn_index.neighbours,
                                                                     std::move(poly)} { }

            explicit PolynomialLocalizingMatrixIndex(::Moment::PolynomialLocalizingMatrixIndex base_index) noexcept
                : PauliPolynomialIndex<Pauli::LocalizingMatrixIndex>{base_index.Level, 0,
                                                                     std::move(base_index.Polynomial)} { }

            [[nodiscard]] std::string to_string(const Context& context, const SymbolTable& symbols) const;
            [[nodiscard]] std::string to_string(const MatrixSystem& system) const;
            [[nodiscard]] static std::string raw_to_string(const Context& context, const SymbolTable& symbols,
                                                           const NearestNeighbourIndex& nn_index,
                                                           const RawPolynomial& raw);
        };

        /**
         * Index of a polynomial commutator matrix in the Pauli scenario.
         */
        struct PolynomialCommutatorMatrixIndex : public PauliPolynomialIndex<Pauli::CommutatorMatrixIndex> {
        public:
            PolynomialCommutatorMatrixIndex(const size_t level, const size_t neighbours, class Polynomial poly) noexcept
                    : PauliPolynomialIndex<Pauli::CommutatorMatrixIndex>{level, neighbours, std::move(poly)} { }

            PolynomialCommutatorMatrixIndex(const NearestNeighbourIndex& nn_index, class Polynomial poly) noexcept
                    : PauliPolynomialIndex<Pauli::CommutatorMatrixIndex>{nn_index.moment_matrix_level,
                                                                         nn_index.neighbours,
                                                                         std::move(poly)} { }

            explicit PolynomialCommutatorMatrixIndex(PolynomialLocalizingMatrixIndex plmi) noexcept
                : PolynomialCommutatorMatrixIndex{plmi.Level, std::move(plmi.Polynomial)} { }


            [[nodiscard]] std::string to_string(const Context& context, const SymbolTable& symbols) const;
            [[nodiscard]] std::string to_string(const MatrixSystem& system) const;
            [[nodiscard]] static std::string raw_to_string(const Context& context, const SymbolTable& symbols,
                                                          const NearestNeighbourIndex& nn_index,
                                                           const RawPolynomial& raw);
        };

        /**
         * Index of a polynomial anti-commutator matrix in the Pauli scenario.
         */
        struct PolynomialAnticommutatorMatrixIndex : public PauliPolynomialIndex<Pauli::AnticommutatorMatrixIndex> {
        public:
            PolynomialAnticommutatorMatrixIndex(const size_t level, const size_t neighbours,
                                                class Polynomial poly) noexcept
                : PauliPolynomialIndex<Pauli::AnticommutatorMatrixIndex>{level, neighbours, std::move(poly)} { }

            PolynomialAnticommutatorMatrixIndex(const NearestNeighbourIndex& nn_index, class Polynomial poly) noexcept
                : PauliPolynomialIndex<Pauli::AnticommutatorMatrixIndex>{nn_index.moment_matrix_level,
                                                                         nn_index.neighbours,
                                                                         std::move(poly)} { }

            explicit PolynomialAnticommutatorMatrixIndex(PolynomialLocalizingMatrixIndex plmi) noexcept
                : PolynomialAnticommutatorMatrixIndex{plmi.Level, std::move(plmi.Polynomial)} { }


            [[nodiscard]] std::string to_string(const Context& context, const SymbolTable& symbols) const;
            [[nodiscard]] std::string to_string(const MatrixSystem& system) const;

            [[nodiscard]] static std::string raw_to_string(const Context& context, const SymbolTable& symbols,
                                                           const NearestNeighbourIndex& nn_index,
                                                           const RawPolynomial& raw);
        };
    }
}