/**
 * implied_map.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
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

        class ImpliedMap {
        private:
            const SymbolTable& origin_symbols;
            const SymbolTable& target_symbols;
            size_t max_length = 0;

            std::vector<SymbolCombo> map;
            std::vector<SymbolCombo> inverse_map;

            std::unique_ptr<MapCore> core;
            std::unique_ptr<SolvedMapCore> core_solution;

        public:

            ImpliedMap(SymmetrizedMatrixSystem& sms, MapCoreProcessor&& proc, const Eigen::MatrixXd& src);

            ImpliedMap(SymmetrizedMatrixSystem& sms, MapCoreProcessor&& proc, const Eigen::SparseMatrix<double>& src);

            ImpliedMap(SymmetrizedMatrixSystem& sms,
                       std::unique_ptr<MapCore> core,
                       std::unique_ptr<SolvedMapCore> solution);

        private:
             void construct_map(const std::vector<symbol_name_t>& osg_to_symbols);

        public:
            ~ImpliedMap() noexcept;

            /**
             * Get symbol/symbol combo in target, associated with symbol in source.
             * @param symbol_id Source symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            [[nodiscard]] const SymbolCombo& operator()(symbol_name_t symbol_id) const;

            /**
              * Create symbol/symbol combo in target, associated with symbol expression in source.
              * Also takes into account prefactors/complex conjugation etc.
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