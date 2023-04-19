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

    namespace Derived {
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

            /**
             * Process MapCore into a SolvedMapCore.
             * Should be logically constant and thread safe.
             */
            virtual std::unique_ptr<SolvedMapCore> operator()(const MapCore& core) const = 0;
        };

        /**
         * Decomposition of raw map into relevant chunks.
         */
        struct MapCore {
        public:
            /** The number of symbols in the origin defined by the map (e.g. columns in initial matrix). */
            size_t initial_size;

            /** Parts of the OSG index that appear in the core as a source. */
            DynamicBitset<size_t> nontrivial_cols;

            /** Parts of the OSG index that appear in the core as a target. */
            DynamicBitset<size_t> nontrivial_rows;

            /** Terms in OSG index that are always ignored (e.g. because they correspond to symbol conjugates). */
            DynamicBitset<size_t> skipped_cols;

            /** Trivial part of the map, from OSG index to constant values */
            std::map<size_t, double> constants;

            /** Constant offset to add to the non-trivial parts of the map. */
            Eigen::RowVectorXd core_offset;

            /** Non-trivial part of the map; each column represents an input; each row an output. */
            Eigen::MatrixXd core;


        public:
            /**
             * Extracts core of map.
             * @param skipped Columns to skip from the transformation matrix.
             * @param matrix The input transformation matrix.
             * @param zero_tolerance The value below which an entry is treated as zero.
             */
            MapCore(DynamicBitset<size_t> skipped, const Eigen::MatrixXd& matrix, double zero_tolerance = 1e-12);

            /**
            * Extracts core of map.
            * @param skipped Columns to skip from the transformation matrix.
            * @param matrix The input transformation sparse matrix.
            */
            MapCore(DynamicBitset<size_t> skipped, const Eigen::SparseMatrix<double>& matrix);

            /**
             * Process map core with visitor class, and sanity check solution.
             * @return Owning pointer to solution.
             */
            [[nodiscard]] inline std::unique_ptr<SolvedMapCore> accept(const MapCoreProcessor& mcp) const {
                return mcp(*this);
            }

            /**
             * Sanity check solution.
             @throws errors::invalid_solution if solution has an obvious problem.
             */
            void check_solution(const SolvedMapCore& solution) const;
        };
    }

}