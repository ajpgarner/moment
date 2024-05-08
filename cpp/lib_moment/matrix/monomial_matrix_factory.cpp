/**
 * monomial_matrix_factory.cpp
 *
 * @copyright Copyright (c) 2023-2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "monomial_matrix_factory.h"
#include "monomial_matrix.h"

#include "symbolic/symbol_table.h"
#include "operator_matrix/operator_matrix.h"
#include "utilities/linear_map_merge.h"


namespace Moment {

    namespace {
        /**
         * Helper class, converts OSM -> Symbol matrix, registering new symbols.
         * Note: this is the single-threaded implementation.
         *
         * @tparam has_prefactor True if constant pre-factor for Monomials should be created
         * @tparam only_hermitian_ops True if every operator is Hermitian
         */
        template<bool has_prefactor, bool only_hermitian_ops = false>
        class OpSeqToSymbolConverter {
        private:
            const Context& context;
            SymbolTable& symbol_table;
            const OperatorMatrix& osm;

        public:
            const bool hermitian = false;
            const std::complex<double> prefactor = {1.0, 1.0};
        public:
            OpSeqToSymbolConverter(const Context& context, SymbolTable& symbol_table,
                                   const OperatorMatrix& osm)
                    : context{context}, symbol_table{symbol_table}, osm{osm}, hermitian{osm.is_hermitian()} {}

            OpSeqToSymbolConverter(const Context& context, SymbolTable& symbol_table,
                                   const OperatorMatrix& osm, const std::complex<double> the_factor)
                    : context{context}, symbol_table{symbol_table}, osm{osm}, hermitian{osm.is_hermitian()},
                      prefactor{the_factor} {}


            std::unique_ptr<SquareMatrix<Monomial>> operator()() {
                auto unique_sequences = hermitian ? identify_unique_sequences_hermitian()
                                                  : identify_unique_sequences_generic();

                symbol_table.merge_in(std::move(unique_sequences));

                return hermitian ? build_symbol_matrix_hermitian()
                                 : build_symbol_matrix_generic();
            }

        private:

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_hermitian() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);

                // Now, look at elements and see if they are unique or not
                auto lower_triangle = osm.LowerTriangle();
                auto iter = lower_triangle.begin();
                const auto iter_end = lower_triangle.end();

                if constexpr (only_hermitian_ops) {
                    while (iter != iter_end) {
                        const auto& conj_elem = *iter;
                        const size_t hash = conj_elem.hash();
                        // Don't add what is already known
                        if (known_hashes.contains(hash)) {
                            ++iter;
                            continue;
                        }

                        // Add hash and symbol
                        known_hashes.emplace(hash);
                        build_unique.emplace_back(Symbol::construct_positive_tag{}, conj_elem);
                    }
                } else {
                    while (iter != iter_end) {
                        // This is a bit of a hack to compensate for col-major storage, while preferring symbols to be
                        // numbered according to the top /row/ of moment matrices, if possible.
                        // Thus, we look at a col-major iterator over the lower triangle, which actually gives us the
                        // conjugates of what were generated; but we define what we find as the conjugate element.
                        const auto& conj_elem = *iter;
                        const auto elem = conj_elem.conjugate();

                        int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            ++iter;
                            continue;
                        }

                        if (elem_hermitian) {
                            build_unique.emplace_back(elem);
                            known_hashes.emplace(hash);
                        } else {
                            if (hash < conj_hash) {
                                build_unique.emplace_back(elem, conj_elem);
                            } else {
                                build_unique.emplace_back(conj_elem, elem);
                            }

                            known_hashes.emplace(hash);
                            known_hashes.emplace(conj_hash);
                        }
                        ++iter;
                    }
                }
                // NRVO?
                return build_unique;
            }

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_generic() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);


                // Now, look at elements and see if they are unique or not
                if constexpr (only_hermitian_ops) {
                    for (const auto& elem: osm) {
                        const size_t hash = elem.hash();
                        // Don't add what is already known
                        if (known_hashes.contains(hash)) {
                            continue;
                        }

                        // Add hash and symbol
                        known_hashes.emplace(hash);
                        build_unique.emplace_back(Symbol::construct_positive_tag{}, elem);
                    }
                } else {
                    // Now, look at elements and see if they are unique or not
                    for (const auto& elem: osm) {

                        const auto conj_elem = elem.conjugate();
                        int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            continue;
                        }

                        if (elem_hermitian) {
                            build_unique.emplace_back(elem);
                            known_hashes.emplace(hash);
                        } else {
                            if (hash < conj_hash) {
                                build_unique.emplace_back(elem, conj_elem);
                            } else {
                                build_unique.emplace_back(conj_elem, elem);
                            }

                            known_hashes.emplace(hash);
                            known_hashes.emplace(conj_hash);
                        }

                    }
                }

                return build_unique;
            }


            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_hermitian() const {

                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);

                // Iterate over upper index
                auto upper_triangle_view = osm.UpperTriangle();
                auto iter = upper_triangle_view.begin();
                const auto iter_end = upper_triangle_view.end();
                while (iter != iter_end) {
                    const size_t row = iter.Row();
                    const size_t col = iter.Col();
                    const auto& elem = *iter;

                    const size_t hash = elem.hash();

                    const auto monomial_sign = to_scalar(elem.get_sign());

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << row << "," << col << "]"
                           << " was not found in symbol table, while parsing Hermitian matrix.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    if constexpr (has_prefactor) {
                        symbolic_representation[iter.Offset()] = Monomial{unique_elem.Id(), prefactor * monomial_sign,
                                                                          conjugated};

                        // Make Hermitian, if off-diagonal
                        if (!iter.diagonal()) {
                            size_t lower_offset = osm.index_to_offset_no_checks(std::array<size_t, 2>{col, row});
                            if (unique_elem.is_hermitian()) {
                                symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                                 prefactor * std::conj(monomial_sign),
                                                                                 false};
                            } else {
                                symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                                 prefactor * std::conj(monomial_sign),
                                                                                 !conjugated};
                            }
                        }
                    } else {
                        symbolic_representation[iter.Offset()] = Monomial{unique_elem.Id(), monomial_sign, conjugated};

                        // Make Hermitian, if off-diagonal
                        if (!iter.diagonal()) {
                            size_t lower_offset = osm.index_to_offset_no_checks(std::array<size_t, 2>{col, row});
                            if (unique_elem.is_hermitian()) {
                                symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                                 std::conj(monomial_sign), false};
                            } else {
                                symbolic_representation[lower_offset] = Monomial{unique_elem.Id(),
                                                                                 std::conj(monomial_sign), !conjugated};
                            }
                        }
                    }
                    ++iter;

                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension, std::move(symbolic_representation));
            }

            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_generic() const {
                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);
                for (size_t offset = 0; offset < osm.ElementCount; ++offset) {
                    const auto& elem = osm[offset];

                    auto elem_factor = to_scalar(elem.get_sign());
                    if constexpr (has_prefactor) {
                        elem_factor *= this->prefactor;
                    }
                    const size_t hash = elem.hash();

                    auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                    if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                        auto index = osm.offset_to_index_no_checks(offset);
                        std::stringstream ss;
                        ss << "Symbol \"" << elem << "\" at index [" << index[0] << "," << index[1] << "]"
                           << " was not found in symbol table.";
                        throw std::logic_error{ss.str()};
                    }
                    const auto& unique_elem = symbol_table[symbol_id];

                    symbolic_representation[offset] = Monomial{unique_elem.Id(), elem_factor, conjugated};
                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension,
                                                                std::move(symbolic_representation));
            }
        };


        [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>>
        do_os_to_sym_st(SymbolTable& symbols, const OperatorMatrix& op_matrix) {
            const auto& context = op_matrix.context;
            if (context.can_be_nonhermitian()) {
                return OpSeqToSymbolConverter<false, false>{context, symbols, op_matrix}();
            } else {
                return OpSeqToSymbolConverter<false, true>{context, symbols, op_matrix}();
            }
        }

        [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>>
        do_os_to_sym_st(SymbolTable& symbols, const OperatorMatrix& op_matrix, const std::complex<double> prefactor) {
            const auto& context = op_matrix.context;
            if (context.can_be_nonhermitian()) {
                return OpSeqToSymbolConverter<true, false>{context, symbols, op_matrix, prefactor}();
            } else {
                return OpSeqToSymbolConverter<true, true>{context, symbols, op_matrix, prefactor}();
            }
        }

    }



    MonomialMatrixFactoryWorker::MonomialMatrixFactoryWorker(MonomialMatrixFactoryMultithreaded& the_bundle,
                                                             const size_t worker_id, const size_t max_workers)
                    : bundle{the_bundle}, worker_id{worker_id}, max_workers{max_workers} {
        assert(worker_id < max_workers);
        assert(max_workers != 0);
        merge_level = std::numeric_limits<size_t>::max();
    }


    void MonomialMatrixFactoryWorker::execute() {

        // Symbol identification
        this->bundle.ready_to_begin_symbol_identification.wait(false, std::memory_order_acquire);
        try {
            this->identify_unique_symbols();
            this->merge_unique_symbols();

            this->done_symbol_identification.set_value(true);
        } catch(...) {
            this->done_symbol_identification.set_exception(std::current_exception());
            if (0 != this->worker_id) {
                // Divide & conquer may be waiting on this...
                this->bundle.workers[0]->done_symbol_identification.set_exception(std::current_exception());
            }
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

    /**
     * First hierarchical level of merge.
     */
    size_t MonomialMatrixFactoryWorker::first_merge_level() const {
        assert(this->max_workers > 0);
        const size_t bf_mw = std::bit_floor(this->max_workers);
        const size_t p = std::bit_width(bf_mw)-1;
        // Trick is to think of division as partitioning the data among threads.
        // Number of workers is a power of two, and so we have 1/2^p of the data in each worker.
        if (bf_mw == this->max_workers) {
            return p;
        }

        // Otherwise, 1/2^p of data in some workers, 1/2^(p+1) in other workers.
        // E.g. 1: N = 10 will have 0, 8 and 1, 9 subdivided to 1/2^4 = 1/16, and all remaining at 1/2^3 = 1/8
        // E.g. 2: N = 5 has 0, 4 subdivided  to 1/2^3 = 1/8, all remaining at 1/2^2 = 1/4

        // Workers above bit floor have 1/2^(p+1) of data
        // E.g. N = 10, bf_mw = 8; so threads 8 and 9 take 1/16.
        if (this->worker_id >= bf_mw) {
            return p+1;
        }

        // Any worker whose first power 2 pair is a thread above the bit floor also has 1/2^(p+1) of data
        if ((this->worker_id + bf_mw) < this->max_workers) { // excess
            return p+1;
        }
        // The remaining workers are not split in half, and have 1/2^(p) of the data.
        return p;
    }

    /**
     * Final hierarchical level of merge.
     */
    constexpr size_t MonomialMatrixFactoryWorker::final_merge_level() const {
        // Thread 0 takes 1/1 of data; 1 takes 1/2, 2 and 3 take 1/4, 4...7 take 1/8, etc.
        return std::bit_width(std::bit_floor(this->worker_id));
    }


    void MonomialMatrixFactoryWorker::identify_unique_symbols_hermitian() {
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
            for (size_t row_idx = col_idx; row_idx < row_length; ++row_idx) {
                const size_t offset = (col_idx * row_length) + row_idx;
                const size_t conj_offset = (row_idx * row_length) + col_idx;
                const auto &elem = this->bundle.os_data_ptr[offset];
                const auto &conj_elem = this->bundle.os_data_ptr[conj_offset];

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
        this->merge_level.store(this->first_merge_level(), std::memory_order_release);
        this->merge_level.notify_all();
    }

    void MonomialMatrixFactoryWorker::identify_unique_symbols_generic() {
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
            for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {
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
        this->merge_level.store(this->first_merge_level(), std::memory_order_release);
        this->merge_level.notify_all();
    }

    void MonomialMatrixFactoryWorker::identify_unique_symbols() {
        // Look for symbols
        if (this->bundle.is_hermitian) {
            this->identify_unique_symbols_hermitian();
        } else {
            this->identify_unique_symbols_generic();
        }
    }

    void MonomialMatrixFactoryWorker::merge_unique_symbols() {
        const size_t final_merge_level = this->final_merge_level();
        while (true) {
            const size_t current_merge_level = this->merge_level.load(std::memory_order_acquire);

            // We are done.
            if (current_merge_level <= final_merge_level) {
                return;
            }
            assert(current_merge_level > 0);

            // Get other worker ID and thread handle
            // e.g. CML=4 pairs with +8; CML=3 pairs with +4; CML=2 pairs with +2; CML=1 pairs with +1
            const size_t wait_for_worker_id = this->worker_id + (1 << (current_merge_level-1));
            assert(wait_for_worker_id < this->max_workers);
            auto& wait_for_worker = *this->bundle.workers[wait_for_worker_id];

            // Wait for other worker to finish up-stream tasks
            // E.g. CML=3: we have 1/8th of the data, so we must wait for the other thread to have this much data.
            size_t other_merge_level = wait_for_worker.merge_level.load(std::memory_order_acquire);
            while (other_merge_level > current_merge_level) {
                wait_for_worker.merge_level.wait(other_merge_level, std::memory_order_relaxed);
                other_merge_level = wait_for_worker.merge_level.load(std::memory_order_acquire);
            }

            // Do merge
            linear_map_merge(this->unique_elements, std::move(wait_for_worker.unique_elements));

            // After merge completed, notify waiting threads that this level of merge has been completed.
            this->merge_level.fetch_sub(1, std::memory_order_release);
            this->merge_level.notify_all();
        }
    }

    void MonomialMatrixFactoryWorker::generate_symbol_matrix() {
        if (this->bundle.is_hermitian) {
            this->generate_symbol_matrix_hermitian();
        } else {
            this->generate_symbol_matrix_generic();
        }
    }

    void MonomialMatrixFactoryWorker::generate_symbol_matrix_generic() {
        assert(this->bundle.os_data_ptr != nullptr);
        assert(this->bundle.sm_data_ptr != nullptr);

        const auto& symbol_table = this->bundle.symbols;
        const auto prefactor = this->bundle.prefactor;
        const size_t row_length = bundle.dimension;

        for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
            for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {

                const size_t offset = (col_idx * row_length) + row_idx;
                const auto& elem = this->bundle.os_data_ptr[offset];

                const auto mono_factor = prefactor * to_scalar(elem.get_sign());
                const size_t hash = elem.hash();
                auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);

                if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                    std::stringstream ss;
                    ss << "Symbol \"" << elem << "\" at index [" << row_idx << "," << col_idx << "]"
                       << " was not found in symbol table.";
                    throw std::logic_error{ss.str()};
                }
                const auto& unique_elem = symbol_table[symbol_id];

                this->bundle.sm_data_ptr[offset] = Monomial{unique_elem.Id(),
                                                            mono_factor,
                                                            conjugated};
            }
        }
    }

    void  MonomialMatrixFactoryWorker::generate_symbol_matrix_hermitian() {
        assert(this->bundle.os_data_ptr != nullptr);
        assert(this->bundle.sm_data_ptr != nullptr);

        const auto& symbol_table = this->bundle.symbols;
        auto * const write_ptr = this->bundle.sm_data_ptr;
        const size_t row_length = bundle.dimension;
        const auto prefactor = this->bundle.prefactor;

        for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
            for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {

                const size_t offset = (col_idx * row_length) + row_idx;
                const size_t trans_offset = (row_idx * row_length) + col_idx;
                const auto& elem = this->bundle.os_data_ptr[offset];

                const size_t hash = elem.hash();
                const auto monomial_sign = to_scalar(elem.get_sign());

                auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);

                if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                    std::stringstream ss;
                    ss << "Symbol \"" << elem << "\" at index [" << row_idx << "," << col_idx << "]"
                       << " was not found in symbol table.";
                    throw std::logic_error{ss.str()};
                }
                const auto& unique_elem = symbol_table[symbol_id];

                // Write entry
                write_ptr[offset] = Monomial{unique_elem.Id(), prefactor * monomial_sign, conjugated};

                // Write H conj entry
                if (offset != trans_offset) {
                    if (unique_elem.is_hermitian()) {
                        write_ptr[trans_offset] = Monomial{unique_elem.Id(),
                                                           prefactor * std::conj(monomial_sign), false};
                    } else {
                        write_ptr[trans_offset] = Monomial{unique_elem.Id(),
                                                           prefactor * std::conj(monomial_sign), !conjugated};
                    }
                }
            }
        }
    }


    MonomialMatrixFactoryMultithreaded::MonomialMatrixFactoryMultithreaded(SymbolTable& symbols,
                                                                           const OperatorMatrix& input_matrix,
                                                                           std::complex<double> prefactor)
        : context{input_matrix.context}, symbols{symbols},
          dimension{input_matrix.Dimension()}, os_data_ptr{input_matrix.raw()}, prefactor{prefactor},
          is_hermitian{input_matrix.is_hermitian()} {

        // Check OS matrix is good
        assert(os_data_ptr != nullptr);


        // Clear progress flags
        this->ready_to_begin_symbol_identification.clear(std::memory_order_relaxed);
        this->ready_to_begin_sm_generation.clear(std::memory_order_relaxed);

        // Query for pool size
        const size_t num_threads = std::min(Multithreading::get_max_worker_threads(), dimension);

        // Set up worker pool
        this->workers.reserve(num_threads);
        for (size_t index = 0; index < num_threads; ++index) {
            // Make new worker
            this->workers.emplace_back(std::make_unique<worker_t>(*this, index, num_threads));

            // Register futures
            auto [si_f, sm_f] = workers.back()->get_futures();
            this->done_symbol_identification.emplace_back(std::move(si_f));
            this->done_sm_generation.emplace_back(std::move(sm_f));
        }

        // Launch threads
        for (auto& worker : workers) {
            worker->launch_thread();
        }
    }

    MonomialMatrixFactoryMultithreaded::~MonomialMatrixFactoryMultithreaded() noexcept {
        for (auto& worker : workers) {
            worker->join();
        }
    }

    std::unique_ptr<SquareMatrix<Monomial>> MonomialMatrixFactoryMultithreaded::execute() {
        // What new symbols are there?
        this->identify_unique_symbols();

        // Add them to the symbol table.
        this->register_unique_symbols();

        // Finally, do symbolization of matrix
        std::vector<Monomial> monomial_data(this->dimension * this->dimension);
        this->sm_data_ptr = monomial_data.data();

        this->generate_symbol_matrix();
        auto mono_m_ptr = std::make_unique<SquareMatrix<Monomial>>(this->dimension, std::move(monomial_data));

        // Clear pointers to (possibly transient) variables:
        this->sm_data_ptr = nullptr;

        // Return newly created monomial matrix
        return mono_m_ptr;
    }

    void MonomialMatrixFactoryMultithreaded::identify_unique_symbols() {
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

    void MonomialMatrixFactoryMultithreaded::register_unique_symbols() {
        // Merge on main thread...
        auto& elems = this->workers.front()->yield_unique_elements();
        this->symbols.merge_in(elems.begin(), elems.end());
    }

    void MonomialMatrixFactoryMultithreaded::generate_symbol_matrix() {
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

    namespace {

        std::unique_ptr<MonomialMatrix>
        register_symbols_and_create_matrix_singlethread(SymbolTable& symbols,
                                                        std::unique_ptr<OperatorMatrix> unaliased_operator_matrix,
                                                        std::unique_ptr<OperatorMatrix> aliased_operator_matrix,
                                                        std::complex<double> prefactor) {
            assert(unaliased_operator_matrix);
            const auto& context = unaliased_operator_matrix->context;

            std::unique_ptr<SquareMatrix<Monomial>> symbolic_matrix;
            if (context.can_have_aliases()) {
                assert(aliased_operator_matrix);

                if (prefactor != 1.0) {

                    symbolic_matrix = do_os_to_sym_st(symbols, *aliased_operator_matrix, prefactor);

                } else {
                    symbolic_matrix = do_os_to_sym_st(symbols, *aliased_operator_matrix);;
                }
            } else {
                assert(aliased_operator_matrix == nullptr);
                if (prefactor != 1.0) {
                    symbolic_matrix = do_os_to_sym_st(symbols, *unaliased_operator_matrix, prefactor);
                } else {
                    symbolic_matrix = do_os_to_sym_st(symbols, *unaliased_operator_matrix);
                }
            }

            return std::make_unique<MonomialMatrix>(symbols,
                                                    std::move(unaliased_operator_matrix),
                                                    std::move(aliased_operator_matrix),
                                                    std::move(symbolic_matrix),
                                                    prefactor);
        }

        std::unique_ptr<MonomialMatrix>
        register_symbols_and_create_matrix_multithread(SymbolTable& symbols,
                                                       std::unique_ptr<OperatorMatrix> unaliased_operator_matrix,
                                                       std::unique_ptr<OperatorMatrix> aliased_operator_matrix,
                                                       std::complex<double> prefactor) {

            assert(unaliased_operator_matrix != nullptr);
            const OperatorMatrix& src_matrix = (aliased_operator_matrix != nullptr)
                                                ? *aliased_operator_matrix
                                                : *unaliased_operator_matrix;


            MonomialMatrixFactoryMultithreaded factory{symbols, src_matrix, prefactor};
            auto symbolic_matrix = factory.execute();

            return std::make_unique<MonomialMatrix>(symbols,
                                                    std::move(unaliased_operator_matrix),
                                                    std::move(aliased_operator_matrix),
                                                    std::move(symbolic_matrix),
                                                    prefactor);
        }
    }

    std::unique_ptr<MonomialMatrix>
    MonomialMatrix::register_symbols_and_create_matrix(SymbolTable& symbols,
            std::unique_ptr<OperatorMatrix> unaliased_operator_matrix,
            std::unique_ptr<OperatorMatrix> aliased_operator_matrix,
            std::complex<double> prefactor,
            Multithreading::MultiThreadPolicy mt_policy) {

        assert(unaliased_operator_matrix != nullptr);
        const size_t numel = unaliased_operator_matrix->Dimension() * unaliased_operator_matrix->Dimension();
        if (Multithreading::should_multithread_matrix_creation(mt_policy, numel)) {
            return register_symbols_and_create_matrix_multithread(symbols, std::move(unaliased_operator_matrix),
                                                                   std::move(aliased_operator_matrix), prefactor);
        } else {
            return register_symbols_and_create_matrix_singlethread(symbols, std::move(unaliased_operator_matrix),
                                                                  std::move(aliased_operator_matrix), prefactor);
        }
    }


}