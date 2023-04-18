/**
 * map_core.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "utilities/dynamic_bitset.h"

#include <Eigen/Core>
#include <Eigen/SparseCore>

namespace Moment {


    class SymbolTable;

    namespace Symmetrized {
        struct MapCore;

        /**
         * Processed MapCore.
         */
        struct SolvedMapCore {
            /** Number of non-constant symbols in range of map (i.e. 'rank' of map). */
            size_t output_symbols = 0;

            /** True if a trivial solution to the map is given.
             * e.g. True if there are no non-constant symbols, or if the map is full rank.
             */
             bool trivial_solution = false;

            /**
              * Map from old core variables to new variables.
              */
             Eigen::MatrixXd map;

             /**
              * Map from new variables back to old core variables.
              */
             Eigen::MatrixXd inv_map;
        };

        /**
         * Visitor class, solves map core to produce transformation.
         */
        class MapCoreProcessor {
        public:
            virtual ~MapCoreProcessor() noexcept = default;
            virtual SolvedMapCore operator()(const MapCore& core) = 0;
        };

        /**
         * Decomposition of raw map into relevant chunks.
         */
        struct MapCore {
        public:
            DynamicBitset<size_t> nontrivial_cols;
            DynamicBitset<size_t> nontrivial_rows;
            std::map<Eigen::Index, double> constants;
            std::set<Eigen::Index> conjugates;
            Eigen::RowVectorXd core_offset;
            Eigen::MatrixXd core;

        public:
            /**
             * Extracts core of map.
             * @param origin_symbols  Associated symbol table [ TODO: Deprecate ]
             * @param matrix The input transformation matrix.
             * @param zero_tolerance The value below which an entry is treated as zero.
             */
            MapCore(const SymbolTable& origin_symbols, const Eigen::MatrixXd& matrix, double zero_tolerance = 1e-12);

            /** Extract core of map from sparse re-write matrix */
            MapCore(const SymbolTable& origin_symbols, const Eigen::SparseMatrix<double>& matrix);

            /** Process map core */
            inline SolvedMapCore accept(MapCoreProcessor& mcp) const {
                return mcp(*this);
            }
        };
    }

}