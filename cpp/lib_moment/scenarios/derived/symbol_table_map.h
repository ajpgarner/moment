/**
 * defining_map.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * The map that defines the symbols of a symmetrized matrix system.
 */

#pragma once

#include "derived_errors.h"

#include "symbolic/polynomial.h"

#include "utilities/dynamic_bitset_fwd.h"
#include "tensor/square_matrix.h"

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>

namespace Moment {
    class Polynomial;
    class SymbolTable;

    namespace Derived {
        class MapCore;
        class MapCoreProcessor;
        class SolvedMapCore;

        class SymbolTableMap {
        protected:
            const SymbolTable& origin_symbols;
            SymbolTable& target_symbols;

            std::vector<Polynomial> map;
            std::vector<Polynomial> inverse_map;
            bool _is_monomial_map = false;

        private:
            std::unique_ptr<MapCore> core;
            std::unique_ptr<SolvedMapCore> core_solution;


        protected:
            /** Partial construction, for derived symbol table maps */
            SymbolTableMap(const SymbolTable& origin, SymbolTable& target);

        public:

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                        const MapCoreProcessor& proc, const Eigen::MatrixXd& src);

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                        const MapCoreProcessor& proc, const Eigen::SparseMatrix<double>& src);

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                       std::unique_ptr<MapCore> core,
                       std::unique_ptr<SolvedMapCore> solution);

        protected:
            /**
             * Use core and solution to build map.
             * @param osg_to_symbols The symbol IDs in the origin table corresponding to the OSG indices.
             * @param cc A bitmap, true if the corresponding OSG index is a complex conjugate of the symbol.
             */
             void construct_map(const std::vector<symbol_name_t>& osg_to_symbols, const DynamicBitset<size_t>& cc);

             /**
              * Write symbols from map to target symbol table.
              */
             void populate_target_symbols();

        public:
            virtual ~SymbolTableMap() noexcept;

            /**
             * Get symbol/symbol combo in target, associated with symbol in source.
             * @param symbol_id Source symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            [[nodiscard]] const Polynomial& operator()(symbol_name_t symbol_id) const;

            /**
              * Create symbol/symbol combo in target, associated with symbol expression in source.
              * Also takes into account pre-factors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] Polynomial operator()(const Monomial& symbol) const;

            /**
              * Create symbol/symbol combo in target, associated with symbol combo in source.
              * Also takes into account pre-factors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] Polynomial operator()(const Polynomial& symbol) const;

            /**
              * Create new polynomial symbolic matrix, mapping source monomial matrix into new symbol set.
              * @param symbol_id Source matrix.
              * @return New symbolic matrix, transforming the supplied input.
              * @throws error::bad_map If any contained symbol is out of range.
              */
            [[nodiscard]] std::unique_ptr<SquareMatrix<Polynomial>>
            operator()(const SquareMatrix<Monomial>& matrix) const;

            /**
              * Create new polynomial symbolic matrix, mapping source polynomial matrix into new symbol set.
              * @param symbol_id Source matrix.
              * @return New symbolic matrix, transforming the supplied input.
              * @throws error::bad_map If any contained symbol is out of range.
              */
            [[nodiscard]] std::unique_ptr<SquareMatrix<Polynomial>>
            operator()(const SquareMatrix<Polynomial>& matrix) const;

            /**
             * Create new monomial symbolic matrix, mapping source monomial matrix into new symbol set.
             * @param symbol_id Source matrix.
             * @return New symbolic matrix, transforming the supplied input.
             * @throws error::bad_map If any contained symbol is out of range, or if map is not monomial.
             */
            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>>
            monomial(const SquareMatrix<Monomial>& matrix) const;

            /**
             * Get symbol/symbol combo in source, associated with symbol in target.
             * @param symbol_id Target symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            [[nodiscard]] const Polynomial& inverse(symbol_name_t symbol_id) const;

            /**
              * Create symbol/symbol combo in source, associated with symbol expression in target.
              * Also takes into account prefactors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] Polynomial inverse(const Monomial& symbol) const;

            /**
             * Number of elements in forward map.
             */
            [[nodiscard]] size_t fwd_size() const noexcept { return this->map.size(); }

            /**
             * True, if map takes monomials in source to monomials in destination (cf. polynomials in destination).
             */
            [[nodiscard]] bool is_monomial_map() const noexcept { return this->_is_monomial_map; }

            /**
             * Number of elements in inverse  map.
             */
            [[nodiscard]] size_t inv_size() const noexcept { return this->inverse_map.size(); }

            /**
             * View core solution directly.
             */
            [[nodiscard]] const SolvedMapCore& raw_solution() const noexcept { return *this->core_solution; }

            [[nodiscard]] const SymbolTable& Origin() const noexcept {
                return this->origin_symbols;
            }

            [[nodiscard]] const SymbolTable& Target() const noexcept {
                return this->target_symbols;
            }


            friend class DerivedMatrixSystem;
        };
    }
}