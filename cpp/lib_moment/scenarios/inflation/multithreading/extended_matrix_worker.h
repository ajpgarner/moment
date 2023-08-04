/**
 * extended_matrix_worker.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "temporary_symbols_and_factors.h"

#include "integer_types.h"

#include "symbolic/monomial.h"
#include "tensor/square_matrix.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <thread>
#include <vector>

namespace Moment {

    namespace Inflation {
        class InflationContext;
        class FactorTable;
    }
    class MomentMatrix;
    class MonomialMatrix;
    class OperatorSequenceGenerator;
    class SymbolTable;

    namespace Multithreading {

        class ExtendedMatrixBundle;

        class ExtendedMatrixWorker {
        public:
            ExtendedMatrixBundle &bundle;
            const size_t worker_id;
            const size_t max_workers;

        private:
            std::thread the_thread;
            Monomial * const output_data;

            /** Thread-local scratch data for combined factors */
            std::vector<symbol_name_t> scratch_combined;

        public:
            ExtendedMatrixWorker(ExtendedMatrixBundle &bundle, size_t worker_id, size_t max_workers);

        public:
            void join() noexcept;

            void launch();

            void execute();

            friend class ExtendedMatrixBundle;

        private:
            symbol_name_t combine_and_register_factors(const std::vector<symbol_name_t>& source_factors,
                                                       const std::vector<symbol_name_t>& extended_factors);

        };

        class ExtendedMatrixBundle {
        public:
            const size_t max_workers;
            const size_t output_dimension;

        private:
            const Inflation::InflationContext &context;

            SymbolTable& symbols;

            TemporarySymbolsAndFactors symbols_and_factors;

            const MonomialMatrix& source_symbols;

            const MomentMatrix& source_operators;

            const std::span<const symbol_name_t> extension_scalars;

            const OperatorSequenceGenerator& source_osg;

            std::vector<Monomial> output_data;


        private:
            std::vector<std::unique_ptr<ExtendedMatrixWorker>> workers;

            /** Mutex for contesting with factor/symbol table writes */
            mutable std::shared_mutex symbol_table_mutex;

        public:
            ExtendedMatrixBundle(const Inflation::InflationContext &context,
                                 SymbolTable &symbols,
                                 Inflation::FactorTable &factors,
                                 const MonomialMatrix &source,
                                 const MomentMatrix &moment_matrix,
                                 const std::span<const symbol_name_t> extension_scalars);

            ~ExtendedMatrixBundle() noexcept;

            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> execute();

            friend class ExtendedMatrixWorker;

        private:
            void prepare_blank_data();

            void join_all() noexcept;
        };

    }
}