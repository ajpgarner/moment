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

#include <memory>

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

            /** True if dense solution provided. */
            bool dense_solution = false;

            /** True if sparse solution provided. */
            bool sparse_solution = false;


            /**
              * Map from old core variables to new variables.
              * If sparse_map is also provided, must be conceptually the same matrix.
              */
             Eigen::MatrixXd dense_map;

             /**
              * Map from new variables back to old core variables.
              * If sparse_inv_map is also provided, must be conceptually the same matrix.
              */
             Eigen::MatrixXd dense_inv_map;

            /**
              * Map from old core variables to new variables.
              * If dense_map is also provided, must be conceptually the same matrix.
              */
             Eigen::SparseMatrix<double> sparse_map;

            /**
             * Map from new variables back to old core variables.
             * If dense_inv_map is also provided, must be conceptually the same matrix.
             */
             Eigen::SparseMatrix<double> sparse_inv_map;

        };

        struct DenseMapCore;
        struct SparseMapCore;

        /**
         * Visitor class, solves map core to produce transformation.
         */
        class MapCoreProcessor {
        public:
            virtual ~MapCoreProcessor() noexcept = default;

            /**
             * Process dense MapCore into a SolvedMapCore.
             * Should be logically constant and thread safe.
             * May throw an exception if dense cores are not supported.
             */
            virtual std::unique_ptr<SolvedMapCore> operator()(const DenseMapCore& core) const = 0;

            /**
             * Process sparse MapCore into a SolvedMapCore.
             * Should be logically constant and thread safe.
             * May throw an exception if sparse cores are not supported.
             */
            virtual std::unique_ptr<SolvedMapCore> operator()(const SparseMapCore& core) const = 0;
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

        public:

            MapCore(size_t initial_src_size, size_t initial_target_size, DynamicBitset<size_t> skipped);

            virtual ~MapCore() = default;

            /**
              * Process map core with visitor class.
              * @return Owning pointer to solution.
              */
            [[nodiscard]] virtual std::unique_ptr<SolvedMapCore> accept(const MapCoreProcessor& mcp) const = 0;

            /**
             * Sanity check solution.
             @throws errors::invalid_solution if solution has an obvious problem.
             */
            virtual void check_solution(const SolvedMapCore& solution) const = 0;

        protected:
            /** Peel off constant/zero columns, and empty rows. */
            std::pair<Eigen::Index, Eigen::Index> identify_nontrivial(const Eigen::MatrixXd& input_dense, double eps_mult=1.0);

            /** Peel off constant/zero columns, and empty rows. */
            std::pair<Eigen::Index, Eigen::Index> identify_nontrivial(const Eigen::SparseMatrix<double>& input_dense);

            static std::vector<Eigen::Index> remap_vector(DynamicBitset<size_t> nontrivial);

            void do_check_solution(size_t outer_rows, size_t outer_cols, const SolvedMapCore& solution) const;
        };


        /**
         * Decomposition of raw map into relevant chunks.
         */
        struct DenseMapCore : public MapCore {
        public:
            /** Non-trivial part of the map; each column represents an input; each row an output. */
            Eigen::MatrixXd core;

        public:
            /**
             * Extracts core of map.
             * @param skipped Columns to skip from the transformation matrix.
             * @param matrix The input transformation matrix.
             * @param zero_eps The multiplier of epsilon, below which an entry is treated as zero.
             */
            DenseMapCore(DynamicBitset<size_t> skipped, const Eigen::MatrixXd& matrix, double zero_eps = 1.0);

            /**
            * Extracts core of map.
            * @param skipped Columns to skip from the transformation matrix.
            * @param matrix The input transformation sparse matrix.
            */
            DenseMapCore(DynamicBitset<size_t> skipped, const Eigen::SparseMatrix<double>& matrix);


            [[nodiscard]] std::unique_ptr<SolvedMapCore> accept(const MapCoreProcessor& mcp) const override {
                return mcp(*this);
            }


            void check_solution(const SolvedMapCore& solution) const override;
        };

        /**
         * Decomposition of raw map into relevant chunks.
         */
        struct SparseMapCore : public MapCore {
        public:
            /** Non-trivial part of the map; each *row* represents an input; each column an output. */
            Eigen::SparseMatrix<double> core;

        public:

            /**
             * Extracts core of map.
             * @param skipped Columns to skip from the transformation matrix.
             * @param matrix The input transformation matrix.
             * @param zero_eps The multiplier of epsilon, below which an entry is treated as zero.
             */
            SparseMapCore(DynamicBitset<size_t> skipped, const Eigen::MatrixXd& matrix, double zero_eps = 1.0);

            /**
            * Extracts core of map.
            * @param skipped Columns to skip from the transformation matrix.
            * @param matrix The input transformation sparse matrix.
            */
            SparseMapCore(DynamicBitset<size_t> skipped, const Eigen::SparseMatrix<double>& matrix);

            /**
             * Process map core with visitor class, and sanity check solution.
             * @return Owning pointer to solution.
             */
            [[nodiscard]] std::unique_ptr<SolvedMapCore> accept(const MapCoreProcessor& mcp) const override {
                return mcp(*this);
            }

            void check_solution(const SolvedMapCore& solution) const override;
        };



    }

}