/**
 * matrix_creation_worker.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence_generator.h"

#include "multithreading.h"

#include "symbolic/symbol_table.h"

#include "utilities/linear_map_merge.h"

#include <atomic>
#include <bit>
#include <future>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <vector>


namespace Moment::Multithreading {

    template<typename elem_functor_t>
    class matrix_generation_worker_bundle;

    template<typename elem_functor_t>
    class matrix_generation_worker {
    private:
        matrix_generation_worker_bundle<elem_functor_t>& bundle;
        std::thread the_thread;

        std::promise<bool> done_os_generation;
        std::promise<bool> done_symbol_identification;
        std::promise<bool> done_sm_generation;

        std::map<size_t, Symbol> unique_elements;

        std::optional<NonHInfo> non_hermitian;

        /** Divide and conquer 'ready' index.
         * Should be changed only by this worker, but read by other workers. */
        std::atomic<size_t> merge_level;

    public:
        const size_t worker_id;
        const size_t max_workers;

        matrix_generation_worker(matrix_generation_worker_bundle<elem_functor_t>& the_bundle,
                                 const size_t worker_id,
                                 const size_t max_workers)
                : bundle{the_bundle}, worker_id{worker_id}, max_workers{max_workers} {
            assert(worker_id < max_workers);
            assert(max_workers != 0);
            merge_level = 0;
        }

        matrix_generation_worker(const matrix_generation_worker& rhs) = delete;

        matrix_generation_worker(matrix_generation_worker&& rhs) = default;

        inline std::tuple<std::future<bool>, std::future<bool>, std::future<bool>>
        get_futures() {
            return std::make_tuple(
                    this->done_os_generation.get_future(),
                    this->done_symbol_identification.get_future(),
                    this->done_sm_generation.get_future()
            );
        }

        inline void launch_thread() {
            this->the_thread = std::thread(&matrix_generation_worker<elem_functor_t>::execute, this);
        }

        [[nodiscard]] std::map<size_t, Symbol>& yield_unique_elements() noexcept {
            return this->unique_elements;
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
                return; // exit thread
            }

            // Symbol identification
            this->bundle.ready_to_begin_symbol_identification.wait(false, std::memory_order_acquire);
            try {
                this->identify_unique_symbols();
                this->merge_unique_symbols();

                this->done_symbol_identification.set_value(true);
            } catch(...) {
                this->done_symbol_identification.set_exception(std::current_exception());
                return; // exit thread
            }


            // Symbol matrix generation
            this->bundle.ready_to_begin_sm_generation.wait(false, std::memory_order_acquire);
            try {
                this->generate_symbol_matrix();
                this->done_sm_generation.set_value(true);
            } catch(...) {
                this->done_sm_generation.set_exception(std::current_exception());
                return; // exit thread
            }

            // Exit thread...
        }


        void generate_operator_sequence_matrix() {
            assert(this->bundle.os_data_ptr != nullptr);
            assert(this->bundle.os_functor != nullptr);

            const size_t row_length = bundle.dimension;

            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                const auto &colSeq = bundle.colGen[col_idx];
                const auto &conjColSeq = bundle.rowGen[col_idx]; // <- Conjugate by construction

                // Diagonal element
                const size_t diag_idx = (col_idx * row_length) + col_idx;
                this->bundle.os_data_ptr[diag_idx] = (*this->bundle.os_functor)(conjColSeq, colSeq);

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
                    const auto &rowSeq = bundle.rowGen[row_idx];
                    const auto &conjRowSeq = bundle.colGen[row_idx]; // <- Conjugate by construction

                    const size_t total_idx = (col_idx * row_length) + row_idx;
                    this->bundle.os_data_ptr[total_idx] = (*this->bundle.os_functor)(rowSeq, colSeq);

                    const size_t conj_idx = (row_idx * row_length) + col_idx;
                    this->bundle.os_data_ptr[conj_idx] = (*this->bundle.os_functor)(conjColSeq, conjRowSeq);

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

        template<bool hermitian>
        void do_identify_unique_symbols() {
            const size_t row_length = bundle.dimension;
            std::set<size_t> known_hashes;

            // First, always manually insert zero and one (if thread 0).
            if (this->worker_id == 0) {
                unique_elements.emplace(static_cast<size_t>(0), Symbol::Zero(this->bundle.context));
                unique_elements.emplace(static_cast<size_t>(1), Symbol::Identity(this->bundle.context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);
            }


            // Now, look at elements and see if they are unique or not
            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                for (size_t row_idx = hermitian ? col_idx : 0; row_idx < row_length; ++row_idx) {
                    const size_t offset = (col_idx * row_length) + row_idx;
                    const auto &elem = this->bundle.os_data_ptr[offset];

                    const auto conj_elem = elem.conjugate();
                    int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                    const bool elem_hermitian = (compare == 1);

                    const size_t hash = elem.hash();
                    const size_t conj_hash = conj_elem.hash();

                    // Don't add what is already known
                    if (known_hashes.contains(hash)) {
                        continue;
                    }

                    if (elem_hermitian) {
                        unique_elements.emplace(hash, elem);
                        known_hashes.emplace(hash);
                    } else {
                        if (hash < conj_hash) {
                            unique_elements.emplace(hash, Symbol{elem, conj_elem});
                        } else {
                            unique_elements.emplace(conj_hash, Symbol{conj_elem, elem});
                        }
                        known_hashes.emplace(hash);
                        known_hashes.emplace(conj_hash);
                    }
                }
            }

            // Flag, that we have calculated level 0, and notify any threads waiting on us.
            this->merge_level.store(1, std::memory_order_release);
            this->merge_level.notify_all();
        }

        void identify_unique_symbols() {
            // Look for symbols
            if (this->bundle.is_hermitian) {
                this->do_identify_unique_symbols<true>();
            } else {
                this->do_identify_unique_symbols<false>();
            }
        }

        void merge_unique_symbols() {
            // Trivially, nothing to do if in right half
            const size_t max_worker_bc = std::bit_ceil(this->max_workers);
            if (this->worker_id >= (max_worker_bc >> 1)) {
                return;
            }
            // Trivially, nothing to do if no paired thread to merge with
            if (this->worker_id + (max_worker_bc >> 1) >= this->max_workers) {
                return;
            }

            while (true) {
                const size_t this_merge_level = this->merge_level.load(std::memory_order_relaxed);
                // We are now done.
                if (this->worker_id >= (max_worker_bc >> this_merge_level)) {
                    return;
                }

                // Get other worker ID
                size_t wait_for_worker_id = this->worker_id + (max_worker_bc >> this_merge_level); // 0 -> 4, 2, 1
                assert(wait_for_worker_id < this->max_workers);


                auto& wait_for_worker = *this->bundle.workers[wait_for_worker_id];

                // Wait for other worker to finish
                size_t other_merge_level = wait_for_worker.merge_level.load(std::memory_order_acquire);
                while (other_merge_level < this_merge_level) {
                    wait_for_worker.merge_level.wait(other_merge_level, std::memory_order_relaxed);
                    other_merge_level = wait_for_worker.merge_level.load(std::memory_order_acquire);
                }

                // Do merge
                linear_map_merge(this->unique_elements, std::move(wait_for_worker.unique_elements));

                // Notify waiting threads that this level of merge has been completed.
                this->merge_level.fetch_add(1, std::memory_order_release);
                this->merge_level.notify_all();
            }
        }


        void generate_symbol_matrix() {
            if (this->bundle.is_hermitian) {
                this->generate_symbol_matrix_hermitian();
            } else {
                this->generate_symbol_matrix_generic();
            }
        }

        void generate_symbol_matrix_generic() {
            assert(this->bundle.os_data_ptr != nullptr);
            assert(this->bundle.sm_data_ptr != nullptr);

            const auto& symbol_table = this->bundle.symbols;

            const size_t row_length = bundle.dimension;
            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {

                    const size_t offset = (col_idx * row_length) + row_idx;
                    const auto& elem = this->bundle.os_data_ptr[offset];

                    const bool negated = elem.negated();
                    const size_t hash = elem.hash();
                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);

                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << row_idx << "," << col_idx << "]"
                           << " was not found in symbol table.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    this->bundle.sm_data_ptr[offset] = Monomial{unique_elem.Id(), negated, conjugated};
                }
            }
        }

        void generate_symbol_matrix_hermitian() {
            assert(this->bundle.os_data_ptr != nullptr);
            assert(this->bundle.sm_data_ptr != nullptr);

            const auto& symbol_table = this->bundle.symbols;
            auto * const write_ptr = this->bundle.sm_data_ptr;

            const size_t row_length = bundle.dimension;
            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {

                    const size_t offset = (col_idx * row_length) + row_idx;
                    const size_t trans_offset = (row_idx * row_length) + col_idx;
                    const auto& elem = this->bundle.os_data_ptr[offset];

                    const bool negated = elem.negated();
                    const size_t hash = elem.hash();
                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);

                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << row_idx << "," << col_idx << "]"
                           << " was not found in symbol table.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    // Write entry
                    write_ptr[offset] = Monomial{unique_elem.Id(), negated, conjugated};

                    // Write H conj entry
                    if (offset != trans_offset) {
                        if (unique_elem.is_hermitian()) {
                            write_ptr[trans_offset] = Monomial{unique_elem.Id(),
                                                                             negated, false};
                        } else {
                            write_ptr[trans_offset] = Monomial{unique_elem.Id(),
                                                                             negated, !conjugated};
                        }
                    }
                }
            }
        }

        void join() {
            assert(this->the_thread.joinable());
            this->the_thread.join();
        }

    };


    template<typename elem_functor_t>
    class matrix_generation_worker_bundle {
    public:
        using worker_t = matrix_generation_worker<elem_functor_t>;

        const Context& context;
        SymbolTable& symbols;

        const OperatorSequenceGenerator &colGen;
        const OperatorSequenceGenerator &rowGen;

        const size_t dimension;

    private:
        std::vector<std::unique_ptr<worker_t>> workers;

        std::vector<std::future<bool>> done_os_generation;
        std::vector<std::future<bool>> done_symbol_identification;
        std::vector<std::future<bool>> done_sm_generation;

        std::atomic_flag ready_to_begin_osm_generation;
        std::atomic_flag ready_to_begin_symbol_identification;
        std::atomic_flag ready_to_begin_sm_generation;



        elem_functor_t * os_functor;
        OperatorSequence * os_data_ptr = nullptr;
        bool is_hermitian = false;
        Monomial * sm_data_ptr = nullptr;
        std::optional<NonHInfo> minimum_non_h_info;

    public:
        friend class matrix_generation_worker<elem_functor_t>;

    public:
        matrix_generation_worker_bundle(const Context& context, SymbolTable& symbols,
                                        const OperatorSequenceGenerator& cols,
                                        const OperatorSequenceGenerator& rows)
              : context{context}, symbols{symbols}, colGen{cols}, rowGen{rows}, dimension{cols.size()} {


            assert(rows.size() == cols.size());
            this->ready_to_begin_osm_generation.clear(std::memory_order_relaxed);
            this->ready_to_begin_symbol_identification.clear(std::memory_order_relaxed);
            this->ready_to_begin_sm_generation.clear(std::memory_order_relaxed);

            const size_t num_threads = Multithreading::get_max_worker_threads();

            // Set up workers, get associated future
            this->workers.reserve(num_threads);
            for (size_t index = 0; index < num_threads; ++index) {
                this->workers.emplace_back(std::make_unique<worker_t>(*this, index, num_threads));
                auto [os_f, si_f, sm_f] = workers.back()->get_futures();
                this->done_os_generation.emplace_back(std::move(os_f));
                this->done_symbol_identification.emplace_back(std::move(si_f));
                this->done_sm_generation.emplace_back(std::move(sm_f));
            }

            // Launch threads
            for (auto& worker : workers) {
                worker->launch_thread();
            }
        }

        matrix_generation_worker_bundle(const matrix_generation_worker_bundle&) = delete;

        /**
         * Destructor: gather finished threads.
         */
        ~matrix_generation_worker_bundle() noexcept {
            for (auto& worker : workers) {
                worker->join();
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

        void generate_operator_sequence_matrix(OperatorSequence * const os_data, elem_functor_t functor) {
            // Supply functor & data pointers
            this->os_data_ptr = os_data;
            this->os_functor = &functor;

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

            this->determine_hermitian_status();

            // Dispose functor
            this->os_functor = nullptr;
        }

        [[nodiscard]] inline std::optional<NonHInfo> non_hermitian_info() const noexcept {
            return this->minimum_non_h_info;
        }


        void identify_unique_symbols() {
            // Signal all workers to begin
            this->ready_to_begin_symbol_identification.test_and_set(std::memory_order_release);
            this->ready_to_begin_symbol_identification.notify_all();

            // Block until all workers are done:
            for (auto& signal : this->done_symbol_identification) {
                bool done = false;
                while (!done) {
                    signal.wait();
                    done = signal.get();
                }
            }
        }


        void register_unique_symbols() {
            // Merge on main thread...
            auto& elems = this->workers.front()->yield_unique_elements();
            this->symbols.merge_in(elems.begin(), elems.end());
        }



        void generate_symbol_matrix(Monomial* symbol_data) {
            // Supply data pointers
            this->sm_data_ptr = symbol_data;

            // Signal all workers to begin
            this->ready_to_begin_sm_generation.test_and_set(std::memory_order_release);
            this->ready_to_begin_sm_generation.notify_all();

            // Block until all workers are done:
            for (auto& signal : this->done_sm_generation) {
                bool done = false;
                while (!done) {
                    signal.wait();
                    done = signal.get();
                }
            }
        }
    };
}