/**
 * monomial_matrix_factory.h
 *
 * Implementation details for making monomial matrices (particularly, multithreaded)
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "monomial_matrix.h"
#include "symbolic/symbol_table.h"

#include <atomic>
#include <future>
#include <thread>
#include <map>


namespace Moment {

    class MonomialMatrixFactoryMultithreaded;

    class MonomialMatrixFactoryWorker {

    private:
        MonomialMatrixFactoryMultithreaded& bundle;
        std::thread the_thread;

        std::promise<bool> done_symbol_identification;
        std::promise<bool> done_sm_generation;

        std::map<size_t, Symbol> unique_elements;


        /** Divide and conquer 'ready' index.
         * Should be changed only by this worker, but read by other workers. */
        std::atomic<size_t> merge_level;

    public:
        const size_t worker_id;
        const size_t max_workers;

        MonomialMatrixFactoryWorker(MonomialMatrixFactoryMultithreaded& the_bundle,
                                    const size_t worker_id, const size_t max_workers);

        MonomialMatrixFactoryWorker(const MonomialMatrixFactoryWorker& rhs) = delete;
        MonomialMatrixFactoryWorker(MonomialMatrixFactoryWorker&& rhs) = delete;

        inline std::tuple<std::future<bool>, std::future<bool>>
        get_futures() {
            return std::make_tuple(
                    this->done_symbol_identification.get_future(),
                    this->done_sm_generation.get_future()
            );
        }

        inline void launch_thread() {
            this->the_thread = std::thread(&MonomialMatrixFactoryWorker::execute, this);
        }

        [[nodiscard]] std::map<size_t, Symbol>& yield_unique_elements() noexcept {
            return this->unique_elements;
        }

        void execute();

        /**
         * First hierarchical level of merge.
         */
        size_t first_merge_level() const;

        /**
         * Final hierarchical level of merge.
         */
        constexpr size_t final_merge_level() const;



        inline void identify_unique_symbols();
        void merge_unique_symbols();

        void generate_symbol_matrix();

    private:
        void identify_unique_symbols_hermitian();
        void identify_unique_symbols_generic();
        void generate_symbol_matrix_generic();
        void generate_symbol_matrix_hermitian();

    public:
        void join() {
            assert(this->the_thread.joinable());
            this->the_thread.join();
        }
    };

    class MonomialMatrixFactoryMultithreaded {
    public:
        using worker_t = MonomialMatrixFactoryWorker;

        /** Operator context. */
        const Context& context;

        /** Symbol table, with write access. */
        SymbolTable& symbols;

        /** Size of matrix. */
        const size_t dimension;

        /** Pointer to operator sequence data. */
        OperatorSequence const * const os_data_ptr;

        /** Multiplicative factor to add in front of all symbols. */
        const std::complex<double> prefactor;

        /** True, if OperatorSequence matrix is Hermitian. */
        const bool is_hermitian;

    private:
        std::vector<std::unique_ptr<worker_t>> workers;

        std::vector<std::future<bool>> done_symbol_identification;
        std::vector<std::future<bool>> done_sm_generation;
        std::atomic_flag ready_to_begin_symbol_identification;
        std::atomic_flag ready_to_begin_sm_generation;


        /** (Transient!) pointer to allocated memory for monomial matrix */
        Monomial * sm_data_ptr = nullptr;
    public:
        friend worker_t;

    public:
        explicit MonomialMatrixFactoryMultithreaded(SymbolTable& symbols,
                                                    const OperatorMatrix& input_matrix,
                                                    const std::complex<double> prefactor);

        /** No copy constructor! */
        MonomialMatrixFactoryMultithreaded(const MonomialMatrixFactoryMultithreaded&) = delete;

        /**
         * Destructor: gather finished threads.
         */
        ~MonomialMatrixFactoryMultithreaded() noexcept;

        /**
         * Do matrix conversion and symbol registration.
         * NB: Only one thread should call execute at once!
         */
        std::unique_ptr<SquareMatrix<Monomial>> execute();

    private:
        void identify_unique_symbols();

        void register_unique_symbols();

        void generate_symbol_matrix();
    };

}