/**
 * multithreaded_operator_matrix_factory.h
 *
 * @copyright Copyright (c) 2023-2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/multithreading.h"

#include "symbolic/symbol_table.h"

#include "utilities/linear_map_merge.h"

#include "is_hermitian.h"

#include <algorithm>
#include <atomic>
#include <bit>
#include <complex>
#include <future>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <vector>


namespace Moment {
    template<typename os_matrix_t, typename context_t, typename index_t, typename functor_t>
    class OperatorMatrixFactory;

    template<typename os_matrix_t, typename context_t, typename index_t, typename elem_functor_t>
    class OperatorMatrixFactoryMultithreaded;

    template<typename os_matrix_t, typename context_t, typename index_t, typename elem_functor_t>
    class OperatorMatrixFactoryWorker {
    public:
        using factory_t = OperatorMatrixFactoryMultithreaded<os_matrix_t, context_t, index_t, elem_functor_t>;
    private:
        factory_t& bundle;
        std::thread the_thread;

        std::promise<bool> done_os_generation;
        std::promise<bool> done_alias_generation;


        std::optional<NonHInfo> non_hermitian;

    public:
        const size_t worker_id;
        const size_t max_workers;

        OperatorMatrixFactoryWorker(factory_t& the_bundle,
                                    const size_t worker_id,
                                    const size_t max_workers)
                : bundle{the_bundle}, worker_id{worker_id}, max_workers{max_workers} {
            assert(worker_id < max_workers);
            assert(max_workers != 0);
        }

        OperatorMatrixFactoryWorker(const OperatorMatrixFactoryWorker& rhs) = delete;

        OperatorMatrixFactoryWorker(OperatorMatrixFactoryWorker&& rhs) = default;

        inline std::tuple<std::future<bool>, std::future<bool>>
        get_futures() {
            return std::make_tuple(
                    this->done_os_generation.get_future(),
                    this->done_alias_generation.get_future()
            );
        }

        inline void launch_thread() {
            this->the_thread = std::thread(
                    &OperatorMatrixFactoryWorker<os_matrix_t, context_t, index_t, elem_functor_t>::execute,
                    this);
        }

        [[nodiscard]] std::optional<NonHInfo> non_hermitian_info() const noexcept {
            return this->non_hermitian;
        };

        void execute() {
            // OSM generation
            this->bundle.ready_to_begin_osm_generation.wait(false, std::memory_order_acquire);
            try {
                this->generate_operator_sequence_matrix();
                this->done_os_generation.set_value(true);
            } catch(...) {
                this->done_os_generation.set_exception(std::current_exception());
                return; // exit thread, having transferred exception to future.
            }

            // Alias generation (if applicable)
            if (this->bundle.factory.context.can_have_aliases()) {
                this->bundle.ready_to_begin_alias_generation.wait(false, std::memory_order_acquire);
                try {
                    this->generate_aliased_operator_sequence_matrix();
                    this->done_alias_generation.set_value(true);
                } catch(...) {
                    this->done_alias_generation.set_exception(std::current_exception());
                    return; // exit thread, having transferred exception to future.
                }
            }

            // Exit thread...
        }

        /**
         * Generates in a manner that assumes the result will be Hermitian
         */
        void generate_operator_sequence_matrix_hermitian() {
            assert(this->bundle.os_data_ptr != nullptr);

            const auto& functor = bundle.factory.elem_functor;
            const auto& col_osg = (*bundle.factory.colGen);
            const auto& row_osg = (*bundle.factory.rowGen);

            const size_t row_length = bundle.factory.dimension;

            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                const auto &colSeq = col_osg[col_idx];

                // Diagonal element
                const size_t diag_idx = (col_idx * row_length) + col_idx;
                const auto &conjColSeq = row_osg[col_idx]; // <- Conjugate by construction
                this->bundle.os_data_ptr[diag_idx] = functor(conjColSeq, colSeq);

                // Off diagonal elements
                for (size_t row_idx = col_idx+1; row_idx < row_length; ++row_idx) {
                    const auto &rowSeq = row_osg[row_idx];

                    const size_t total_idx = (col_idx * row_length) + row_idx;
                    this->bundle.os_data_ptr[total_idx] = functor(rowSeq, colSeq);

                    const size_t conj_idx = (row_idx * row_length) + col_idx;
                    this->bundle.os_data_ptr[conj_idx] = this->bundle.os_data_ptr[total_idx].conjugate();
                }
            }
        }

        /**
         * Generates in a manner that must test if the result is be Hermitian.
         */
        void generate_operator_sequence_matrix_generic() {
            assert(this->bundle.os_data_ptr != nullptr);

            const auto& functor = bundle.factory.elem_functor;
            const auto& col_osg = (*bundle.factory.colGen);
            const auto& row_osg = (*bundle.factory.rowGen);

            const size_t row_length = bundle.factory.dimension;

            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                const auto &colSeq = col_osg[col_idx];
                const auto &conjColSeq = row_osg[col_idx]; // <- Conjugate by construction

                // Diagonal element
                const size_t diag_idx = (col_idx * row_length) + col_idx;
                this->bundle.os_data_ptr[diag_idx] = functor(conjColSeq, colSeq);

                // Check for Hermiticity if not yet non-hermitian
                if (!this->non_hermitian.has_value()) {
                    const auto& seq = this->bundle.os_data_ptr[diag_idx];
                    const auto conj_seq = seq.conjugate();
                    if (seq.hash() != conj_seq.hash()) {
                        this->non_hermitian.emplace(col_idx, col_idx);
                    }
                }

                // Off diagonal elements
                for (size_t row_idx = col_idx+1; row_idx < row_length; ++row_idx) {
                    const auto &rowSeq = row_osg[row_idx];
                    const auto &conjRowSeq = col_osg[row_idx]; // <- Conjugate by construction

                    const size_t total_idx = (col_idx * row_length) + row_idx;
                    this->bundle.os_data_ptr[total_idx] = functor(rowSeq, colSeq);

                    const size_t conj_idx = (row_idx * row_length) + col_idx;
                    this->bundle.os_data_ptr[conj_idx] = functor(conjColSeq, conjRowSeq);

                    // Check for Hermiticity if not yet nonhermitian
                    if (!this->non_hermitian.has_value()) {
                        const auto& seq = this->bundle.os_data_ptr[total_idx];
                        const auto conj_seq = seq.conjugate();
                        const auto& tx_seq = this->bundle.os_data_ptr[conj_idx];
                        if (conj_seq.hash() != tx_seq.hash()) {
                            this->non_hermitian.emplace(row_idx, col_idx);
                        }
                    }
                }
            }
        }

        inline void generate_operator_sequence_matrix() {
            if (this->bundle.factory.could_be_non_hermitian) {
                generate_operator_sequence_matrix_generic();
            } else {
                generate_operator_sequence_matrix_hermitian();
            }
        }

        void generate_aliased_operator_sequence_matrix_generic() {
            assert(this->bundle.os_data_ptr != nullptr);
            assert(this->bundle.alias_data_ptr != nullptr);

            // Reset non-Hermitian information
            this->non_hermitian.reset();

            const auto& context = this->bundle.factory.context;
            const size_t row_length = bundle.factory.dimension;
            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {

                // Diagonal element
                const size_t diag_idx = (col_idx * row_length) + col_idx;
                this->bundle.alias_data_ptr[diag_idx] =
                        context.simplify_as_moment(this->bundle.os_data_ptr[diag_idx]);

                // Check for Hermiticity if not yet non-hermitian
                if (!this->non_hermitian.has_value()) {
                    const auto& seq = this->bundle.os_data_ptr[diag_idx];
                    const auto conj_seq = seq.conjugate();
                    if (seq.hash() != conj_seq.hash()) {
                        this->non_hermitian.emplace(col_idx, col_idx);
                    }
                }

                for (size_t row_idx = col_idx+1; row_idx < row_length; ++row_idx) {
                    const size_t total_idx = (col_idx * row_length) + row_idx;
                    const size_t conj_idx  = (row_idx * row_length) + col_idx;

                    this->bundle.alias_data_ptr[total_idx] =
                            context.simplify_as_moment(this->bundle.os_data_ptr[total_idx]);
                    this->bundle.alias_data_ptr[conj_idx] =
                            context.simplify_as_moment(this->bundle.os_data_ptr[conj_idx]);

                    // Check for Hermiticity if not yet non-Hermitian
                    if (!this->non_hermitian.has_value()) {
                        const auto& seq = this->bundle.os_data_ptr[total_idx];
                        const auto conj_seq = seq.conjugate();
                        const auto& tx_seq = this->bundle.os_data_ptr[conj_idx];
                        if (conj_seq.hash() != tx_seq.hash()) {
                            this->non_hermitian.emplace(row_idx, col_idx);
                        }
                    }
                }
            }
        }


        void generate_aliased_operator_sequence_matrix_hermitian() {
            assert(this->bundle.os_data_ptr != nullptr);
            assert(this->bundle.alias_data_ptr != nullptr);

            const auto& context = bundle.factory.context;

            const size_t row_length = bundle.factory.dimension;

            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {

                // Diagonal element
                const size_t diag_idx = (col_idx * row_length) + col_idx;
                this->bundle.alias_data_ptr[diag_idx] =
                        context.simplify_as_moment(this->bundle.os_data_ptr[diag_idx]);

                for (size_t row_idx = col_idx+1; row_idx < row_length; ++row_idx) {
                    const size_t total_idx = (col_idx * row_length) + row_idx;
                    const size_t conj_idx  = (row_idx * row_length) + col_idx;

                    this->bundle.alias_data_ptr[total_idx] =
                            context.simplify_as_moment(OperatorSequence{this->bundle.os_data_ptr[total_idx]});
                    // simplify_as_moment must commute with Hermitian conjugation:
                    this->bundle.alias_data_ptr[conj_idx] = this->bundle.alias_data_ptr[total_idx].conjugate();
                }
            }
        }


        void generate_aliased_operator_sequence_matrix() {
            if (this->bundle.factory.could_be_non_hermitian) {
                generate_aliased_operator_sequence_matrix_generic();
            } else {
                generate_aliased_operator_sequence_matrix_hermitian();
            }
        }

        void join() {
            assert(this->the_thread.joinable());
            this->the_thread.join();
        }
    };

    template<typename os_matrix_t, typename context_t, typename index_t, typename elem_functor_t>
    class OperatorMatrixFactoryMultithreaded {
    public:
        using info_factory_t = OperatorMatrixFactory<os_matrix_t, context_t, index_t, elem_functor_t>;
        using worker_t = OperatorMatrixFactoryWorker<os_matrix_t, context_t, index_t, elem_functor_t>;

    private:
        info_factory_t& factory;

        std::vector<std::unique_ptr<worker_t>> workers;

        std::vector<std::future<bool>> done_os_generation;
        std::vector<std::future<bool>> done_alias_generation;

        std::atomic_flag ready_to_begin_osm_generation;
        std::atomic_flag ready_to_begin_alias_generation;

        /** Pointer to allocated memory for operator sequence matrix. */
        OperatorSequence * os_data_ptr = nullptr;

        /** Pointer to allocated memory for aliased operator sequence matrix. */
        OperatorSequence * alias_data_ptr = nullptr;

        /** True if, in principle, the generation requested could make a non-Hermitian matrix. */
        bool could_be_non_hermitian = true;

        /** True if, in actuality, the generation requested made a non-Hermitian matrix. */
        bool is_hermitian = false;

        /** Optional information about non-Hermitian element in matrix. */
        std::optional<NonHInfo> minimum_non_h_info;

    public:
        friend worker_t;

    public:
        explicit OperatorMatrixFactoryMultithreaded(info_factory_t& factory)
            : factory{factory} {

            // Clear progress flags
            this->ready_to_begin_osm_generation.clear(std::memory_order_relaxed);
            this->ready_to_begin_alias_generation.clear(std::memory_order_relaxed);

            // Query for pool size
            const size_t num_threads = std::min(Multithreading::get_max_worker_threads(), factory.dimension);

            // Set up worker pool
            this->workers.reserve(num_threads);
            for (size_t index = 0; index < num_threads; ++index) {
                // Make new worker
                this->workers.emplace_back(std::make_unique<worker_t>(*this, index, num_threads));

                // Register futures
                auto [os_f, aos_f] = workers.back()->get_futures();
                this->done_os_generation.emplace_back(std::move(os_f));
                this->done_alias_generation.emplace_back(std::move(aos_f));
            }

            // Launch threads
            for (auto& worker : workers) {
                worker->launch_thread();
            }
        }

        /** No copy constructor! */
        OperatorMatrixFactoryMultithreaded(const OperatorMatrixFactoryMultithreaded&) = delete;

        /**
         * Destructor: gather finished threads.
         */
        ~OperatorMatrixFactoryMultithreaded() noexcept {
            for (auto& worker : workers) {
                worker->join();
            }
        }

        /**
         * Make operator matrices
         * NB: Only one thread should call execute at once!
         */
        std::pair<std::unique_ptr<os_matrix_t>, std::unique_ptr<os_matrix_t>>
        make_aliased() {
            // Allocate memory for unaliased operator sequences
            const size_t numel = this->factory.dimension * this->factory.dimension;
            std::vector<OperatorSequence> unaliased_data{OperatorSequence::create_uninitialized_vector(numel)};

            this->os_data_ptr = unaliased_data.data();
            this->generate_operator_sequence_matrix();

            // Create operator sequence square matrix, with aliased data, and knowledge of Hermiticity
            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(factory.dimension, std::move(unaliased_data),
                                                                     this->non_hermitian_info());

            // Create OSM
            auto unaliased_operator_matrix = std::make_unique<os_matrix_t>(factory.context, factory.Index,
                                                                           std::move(OSM));
            // Likely same as before; promise not to change from here on!
            this->os_data_ptr = const_cast<OperatorSequence *>(unaliased_operator_matrix->raw());

            // Do we need to detect aliasing?
            std::unique_ptr<os_matrix_t> aliased_operator_matrix;
            std::vector<OperatorSequence> aliased_data;
            if (this->factory.context.can_have_aliases()) {
                aliased_data = OperatorSequence::create_uninitialized_vector(numel);
                this->alias_data_ptr = aliased_data.data();
                std::atomic_thread_fence(std::memory_order_release);
                this->generate_aliased_operator_sequence_matrix();

                auto aOSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(factory.dimension,
                                                                          std::move(aliased_data),
                                                                          this->non_hermitian_info());
                aliased_operator_matrix = std::make_unique<os_matrix_t>(factory.context, factory.Index,
                                                                        std::move(aOSM));

                // Likely same as before; promise not to change from here on!
                this->alias_data_ptr = const_cast<OperatorSequence *>(aliased_operator_matrix->raw());
            } else {
                // No aliasing, so 'aliased' (really, de-aliased) operators are just the raw operators.
                this->alias_data_ptr = this->os_data_ptr;
            }

            return {std::move(unaliased_operator_matrix), std::move(aliased_operator_matrix)};
        }


        /**
         * Make operator matrix
         * NB: Only one thread should call execute at once!
         */
        std::unique_ptr<os_matrix_t> make_unaliased() {
            // Allocate memory for unaliased operator sequences
            const size_t numel = this->factory.dimension * this->factory.dimension;
            std::vector<OperatorSequence> unaliased_data{OperatorSequence::create_uninitialized_vector(numel)};

            this->os_data_ptr = unaliased_data.data();
            this->generate_operator_sequence_matrix();

            // Create operator sequence square matrix, with aliased data, and knowledge of Hermiticity
            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(factory.dimension, std::move(unaliased_data),
                                                                     this->non_hermitian_info());

            // Create OSM
            auto unaliased_operator_matrix = std::make_unique<os_matrix_t>(factory.context, factory.Index,
                                                                           std::move(OSM));
            // Likely same as before; promise not to change from here on!
            this->os_data_ptr = const_cast<OperatorSequence *>(unaliased_operator_matrix->raw());

            return unaliased_operator_matrix;
        }

    private:
        void generate_operator_sequence_matrix() {

            // Signal all workers to begin
            this->ready_to_begin_osm_generation.test_and_set(std::memory_order_release);
            this->ready_to_begin_osm_generation.notify_all();

            // Block until all workers are done:
            for (auto& signal : this->done_os_generation) {
                bool done = false;
                while (!done) {
                    signal.wait();
                    done = signal.get();
                }
            }

            // Leading thread determines if constructed matrix is Hermitian.
            this->determine_hermitian_status();
        }

        void generate_aliased_operator_sequence_matrix() {
            // Signal all workers to begin
            this->ready_to_begin_alias_generation.test_and_set(std::memory_order_release);
            this->ready_to_begin_alias_generation.notify_all();

            // Block until all workers are done:
            for (auto& signal : this->done_alias_generation) {
                bool done = false;
                while (!done) {
                    signal.wait();
                    done = signal.get();
                }
            }
        }

        void determine_hermitian_status() {
            // Query workers, and find lowest non-Hermitian element
            const NonHInfoOrdering nh_less{};
            this->minimum_non_h_info = std::nullopt;
            for (const auto& worker_ptr : this->workers) {
                const auto& nhi = worker_ptr->non_hermitian_info();
                if (nh_less(nhi, this->minimum_non_h_info)) {
                    this->minimum_non_h_info = nhi;
                }
            }

            this->is_hermitian = !this->minimum_non_h_info.has_value();
        }

        [[nodiscard]] inline std::optional<NonHInfo> non_hermitian_info() const noexcept {
            return this->minimum_non_h_info;
        }
    };
}