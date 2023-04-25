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

#include "symbolic/symbol_combo.h"

#include "utilities/dynamic_bitset.h"
#include "utilities/square_matrix.h"

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
    class SymbolCombo;
    class SymbolTable;

    namespace Derived {
        class MapCore;
        class MapCoreProcessor;
        class SolvedMapCore;

        class SymbolTableMap {
        private:
            const SymbolTable& origin_symbols;
            SymbolTable& target_symbols;

            std::vector<SymbolCombo> map;
            std::vector<SymbolCombo> inverse_map;

            std::unique_ptr<MapCore> core;
            std::unique_ptr<SolvedMapCore> core_solution;

        public:

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                        const MapCoreProcessor& proc, const Eigen::MatrixXd& src);

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                        const MapCoreProcessor& proc, const Eigen::SparseMatrix<double>& src);

            SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                       std::unique_ptr<MapCore> core,
                       std::unique_ptr<SolvedMapCore> solution);

        private:
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
            ~SymbolTableMap() noexcept;

            /**
             * Get symbol/symbol combo in target, associated with symbol in source.
             * @param symbol_id Source symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            [[nodiscard]] const SymbolCombo& operator()(symbol_name_t symbol_id) const;

            /**
              * Create symbol/symbol combo in target, associated with symbol expression in source.
              * Also takes into account pre-factors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] SymbolCombo operator()(const SymbolExpression& symbol) const;

            /**
              * Create symbol/symbol combo in target, associated with symbol combo in source.
              * Also takes into account pre-factors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] SymbolCombo operator()(const SymbolCombo& symbol) const;

            /**
              * Create new polynomial symbolic matrix, mapping source monomial matrix into new symbol set.
              * @param symbol_id Source matrix.
              * @return New symbolic matrix, transforming the supplied input.
              * @throws error::bad_map If any contained symbol is out of range.
              */
            [[nodiscard]] std::unique_ptr<SquareMatrix<SymbolCombo>>
            operator()(const SquareMatrix<SymbolExpression>& matrix) const;

            /**
              * Create new polynomial symbolic matrix, mapping source polynomial matrix into new symbol set.
              * @param symbol_id Source matrix.
              * @return New symbolic matrix, transforming the supplied input.
              * @throws error::bad_map If any contained symbol is out of range.
              */
            [[nodiscard]] std::unique_ptr<SquareMatrix<SymbolCombo>>
            operator()(const SquareMatrix<SymbolCombo>& matrix) const;

            /**
             * Get symbol/symbol combo in source, associated with symbol in target.
             * @param symbol_id Target symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            [[nodiscard]] const SymbolCombo& inverse(symbol_name_t symbol_id) const;

            /**
              * Create symbol/symbol combo in source, associated with symbol expression in target.
              * Also takes into account prefactors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            [[nodiscard]] SymbolCombo inverse(const SymbolExpression& symbol) const;

            /**
             * Number of elements in forward map.
             */
            [[nodiscard]] size_t fwd_size() const noexcept { return this->map.size(); }

            /**
             * Number of elements in inverse  map.
             */
            [[nodiscard]] size_t inv_size() const noexcept { return this->inverse_map.size(); }

            /**
             * View core solution directly.
             */
            [[nodiscard]] const SolvedMapCore& raw_solution() const noexcept { return *this->core_solution; }


        };
    };

}