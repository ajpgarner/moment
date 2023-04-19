/**
 * defining_map.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * The map that defines the symbols of a symmetrized matrix system.
 */

#pragma once

#include "symmetrized_errors.h"
#include "representation.h"

#include "symbolic/symbol_combo.h"

#include "utilities/dynamic_bitset.h"

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

    namespace Symmetrized {
        class Representation;
        class SymmetrizedMatrixSystem;
        class MapCore;
        class MapCoreProcessor;
        class SolvedMapCore;

        class DefiningMap {
        private:
            const SymbolTable& origin_symbols;
            SymbolTable& target_symbols;
            size_t max_length = 0;

            std::vector<SymbolCombo> map;
            std::vector<SymbolCombo> inverse_map;

            std::unique_ptr<MapCore> core;
            std::unique_ptr<SolvedMapCore> core_solution;

        public:

            DefiningMap(const SymbolTable& origin, SymbolTable& target,
                        MapCoreProcessor&& proc, const Eigen::MatrixXd& src);

            DefiningMap(const SymbolTable& origin, SymbolTable& target,
                        MapCoreProcessor&& proc, const Eigen::SparseMatrix<double>& src);

            DefiningMap(const SymbolTable& origin, SymbolTable& target,
                       std::unique_ptr<MapCore> core,
                       std::unique_ptr<SolvedMapCore> solution);

        private:
            /**
             * Use core and solution to build map.
             * @param osg_to_symbols The symbol IDs in the origin table corresponding to the OSG indices.
             */
             void construct_map(const std::vector<symbol_name_t>& osg_to_symbols);

             /**
              *
              * @return Number of defined symbols.
              */
             size_t populate_target_symbols();

        public:
            ~DefiningMap() noexcept;

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
             * Get symbol/symbol combo in source, associated with symbol in target
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
             * The longest word that can be remapped by this map.
             */
             [[nodiscard]] size_t longest_word() const noexcept { return this->max_length; }

        };
    };

}