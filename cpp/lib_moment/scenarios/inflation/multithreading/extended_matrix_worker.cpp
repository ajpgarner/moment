/**
 * extended_matrix_worker.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "extended_matrix_worker.h"
#include "scenarios/inflation/extended_matrix.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/factor_table.h"

#include "dictionary/operator_sequence_generator.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace Moment::Multithreading {

    using namespace Moment::Inflation;

    ExtendedMatrixWorker::ExtendedMatrixWorker(ExtendedMatrixBundle& bundle,
                                               const size_t worker_id, const size_t max_workers)
            : bundle{bundle}, worker_id{worker_id}, max_workers{max_workers},
            the_thread{}, output_data{bundle.output_data.data()} {
        assert(worker_id < max_workers);
    }

    void ExtendedMatrixWorker::launch() {
        assert(!this->the_thread.joinable());
        this->the_thread = std::thread{&ExtendedMatrixWorker::execute, this};
    }

    void ExtendedMatrixWorker::execute() {
        const auto * const src_data = this->bundle.source_symbols.raw_data();
        const size_t src_dimension = this->bundle.source_symbols.Dimension();
        const size_t full_dimension = this->bundle.output_dimension;

        // Thread-local scratch assignment
        this->scratch_combined.reserve(10);

        // Step 1. Copy existing data
        for (size_t col = this->worker_id; col < src_dimension; col += max_workers) {
            const size_t input_col_offset = col * src_dimension;
            const size_t output_col_offset = col * full_dimension;

            // Copy existing row
            std::copy(src_data + input_col_offset, src_data + input_col_offset + src_dimension,
                      output_data + output_col_offset);

            // Get symbol for column
            const size_t source_op_hash = this->bundle.context.simplify_as_moment(
                    OperatorSequence{this->bundle.source_osg[col]}
            ).hash();

            // All new symbols created during this process do not have hashes, therefore this is not a race condition.
            auto [source_symbol_id, source_conj] = this->bundle.symbols.hash_to_index(source_op_hash);
            assert(!source_conj);

            // First, get factors from column
            const auto& col_factors = this->bundle.symbols_and_factors.find_factors_by_symbol_id(source_symbol_id);

            // Extensions for sides (and by symmetry, bottom)
            for (size_t row = src_dimension; row < full_dimension; ++row) {
                const symbol_name_t extension_symbol_id = this->bundle.extension_scalars[row - src_dimension];

                // Get factors from extension, and combine
                const auto& row_factors = this->bundle.symbols_and_factors.find_factors_by_symbol_id(extension_symbol_id);
                symbol_name_t combined_id = this->combine_and_register_factors(col_factors, row_factors);

                // Write symmetrically to symbol matrix
                *(this->output_data + output_col_offset + row) = Monomial{combined_id};
                *(this->output_data + (full_dimension * row) + col) = Monomial{combined_id};
            }
        }

        //  Do extensions for lower right block
        for (size_t col = src_dimension + this->worker_id; col < full_dimension; col += max_workers) {
            const size_t output_col_offset = col * full_dimension;

            const symbol_name_t extension_symbol_col_id = this->bundle.extension_scalars[col - src_dimension];
            const auto& col_factors = this->bundle.symbols_and_factors.find_factors_by_symbol_id(extension_symbol_col_id);
            auto combined_diagonal_id = this->combine_and_register_factors(col_factors, col_factors);

            *(this->output_data + output_col_offset + col) = Monomial{combined_diagonal_id};

            for (size_t row = col+1; row < full_dimension; ++row) {
                const symbol_name_t extension_symbol_row_id = this->bundle.extension_scalars[row - src_dimension];
                const auto& row_factors = this->bundle.symbols_and_factors.find_factors_by_symbol_id(extension_symbol_row_id);
                auto combined_off_diagonal_id = this->combine_and_register_factors(col_factors, row_factors);

                // Write symmetrically
                *(this->output_data + output_col_offset + row) = Monomial{combined_off_diagonal_id};
                *(this->output_data + (row * full_dimension) + col) = Monomial{combined_off_diagonal_id};
            }
        }
    }

    void ExtendedMatrixWorker::join() noexcept {
        if (this->the_thread.joinable()) {
            this->the_thread.join();
        }
    }

    symbol_name_t
    ExtendedMatrixWorker::combine_and_register_factors(const std::vector<symbol_name_t>& source_factors,
                                                       const std::vector<symbol_name_t>& extended_factors) {
        FactorTable::combine_symbolic_factors(this->scratch_combined, source_factors, extended_factors);
        return this->bundle.symbols_and_factors.find_or_register_factors(this->scratch_combined);
    }

    ExtendedMatrixBundle::ExtendedMatrixBundle(const InflationContext &context, SymbolTable &symbols,
                                               FactorTable &factors, const MonomialMatrix &source,
                                               const MomentMatrix &moment_matrix,
                                               const std::span<const symbol_name_t> extension_scalars)
           : max_workers{Multithreading::get_max_worker_threads()},
             context{context},  symbols{symbols}, symbols_and_factors{symbols, factors},
             source_symbols{source}, source_operators{moment_matrix}, extension_scalars{extension_scalars},
             output_dimension{source.Dimension() + extension_scalars.size()},
             source_osg{context.operator_sequence_generator(moment_matrix.Level(), false)} {

        assert(source_osg.size() == source.Dimension());
        // Create output data
        this->prepare_blank_data();

        // Create workers
        this->workers.reserve(max_workers);
        for (size_t worker_id = 0; worker_id < max_workers; ++worker_id) {
            this->workers.emplace_back(std::make_unique<ExtendedMatrixWorker>(*this, worker_id, max_workers));
        }
    }

    ExtendedMatrixBundle::~ExtendedMatrixBundle() noexcept {
        this->join_all();
    }

    std::unique_ptr<SquareMatrix<Monomial>> ExtendedMatrixBundle::execute() {
        for (auto& worker_ptr : this->workers) {
            assert(worker_ptr);
            worker_ptr->launch();
        }

        this->join_all();

        this->symbols_and_factors.register_new_symbols_and_factors();

        // Move output data
        return std::make_unique<SquareMatrix<Monomial>>(this->output_dimension, std::move(this->output_data));
    }

    void ExtendedMatrixBundle::prepare_blank_data() {
        const size_t numel = this->output_dimension * this->output_dimension;
        this->output_data.assign(numel, Monomial{});
    }

    void ExtendedMatrixBundle::join_all() noexcept {
        for (auto& worker_ptr : this->workers) {
            if (worker_ptr) {
                worker_ptr->join();
            }
        }
    }



}