/**
 * group_rep_generation_worker.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "group_rep_generation_worker.h"

#include "representation.h"
#include "representation_mapper.h"

#include <cassert>

#include <algorithm>

namespace Moment::Multithreading {
    using namespace Moment::Symmetrized;

    GroupRepGenerationWorker::GroupRepGenerationWorker(GroupRepGenerationBundle& bundle,
                                                       size_t worker_id, size_t max_workers)
        : bundle{bundle}, worker_id{worker_id}, max_workers{max_workers}, the_thread{}, last_generated_index{0} {
        assert(worker_id < max_workers);
    }

    void GroupRepGenerationWorker::launch() {
        assert(!this->the_thread.joinable());
        this->the_thread = std::thread{&GroupRepGenerationWorker::execute, this};
    }

    void GroupRepGenerationWorker::execute() {

        // Wait until it is time to work:
        size_t global_index = 0;
        size_t local_index = this->last_generated_index.load(std::memory_order_relaxed);
        const size_t max_index = this->bundle.build_list.size();

        // Loop through sub-representations
        while (local_index < max_index) {

            // Wait until bundle signals it is time to work
            while (global_index < local_index) {
                this->bundle.global_index.wait(global_index, std::memory_order_relaxed);
                global_index = this->last_generated_index.load(std::memory_order_acquire);
            }

            // Get parents, which are guaranteed after wait, to exist
            assert(local_index < this->bundle.build_list.size());
            const size_t wl = this->bundle.build_list[local_index];
            const auto& [left_parent, right_parent] =
                    Group::determine_parent_representations(this->bundle.representations, wl);

            // Get mapper
            assert(((wl-1) < this->bundle.mappers.size()) && this->bundle.mappers[wl-1]);
            const auto& mapper = *this->bundle.mappers[wl-1];

            // Get data pointer
            assert(local_index < this->bundle.rep_raw_data.size());
            assert(this->bundle.rep_raw_data[local_index].size() == this->bundle.group_size);
            repmat_t* const data_ptr = this->bundle.rep_raw_data[local_index].data();

            // Make elements for next representation
            for (size_t elem_idx = this->worker_id; elem_idx < this->bundle.group_size; elem_idx += this->max_workers) {
                data_ptr[elem_idx] = mapper(left_parent[elem_idx], right_parent[elem_idx]);
            }

            // Signal that work is done for this level
            this->last_generated_index.fetch_add(1, std::memory_order_release);
            local_index = this->last_generated_index.load(std::memory_order_relaxed);
            this->last_generated_index.notify_all();
        }
    }

    void GroupRepGenerationWorker::join() noexcept {
        if (this->the_thread.joinable()) {
            this->the_thread.join();
            this->last_generated_index.store(0, std::memory_order_release);
        }
    }

    GroupRepGenerationBundle::GroupRepGenerationBundle(
            std::vector<std::unique_ptr<Symmetrized::Representation>>& representations,
            std::vector<std::unique_ptr<Symmetrized::RepresentationMapper>>& mappers, const size_t group_size,
            Group::build_list_t build_list)
            : max_workers{std::min(Multithreading::get_max_worker_threads(), group_size)},
              group_size{group_size}, build_list(std::move(build_list)),
              representations{representations}, mappers{mappers},
              global_index{0} {

        // Create output data
        this->prepare_blank_data();

        // Create workers
        this->workers.reserve(max_workers);
        for (size_t worker_id = 0; worker_id < max_workers; ++worker_id) {
            this->workers.emplace_back(std::make_unique<GroupRepGenerationWorker>(*this, worker_id, max_workers));
        }
    }

    void GroupRepGenerationBundle::execute() {
        // Set to index 0
        this->global_index.store(0, std::memory_order_release);

        // Launch threads
        for (auto& worker_ptr : this->workers) {
            assert(worker_ptr);
            worker_ptr->launch();
        }


        size_t g_idx = this->global_index.load(std::memory_order_relaxed);
        for (const auto wl : build_list) {
            // Should have been pruned
            assert(!this->representations[wl-1]);

            // Wait for threads to have generated level
            for (auto& worker_ptr : this->workers) {
                size_t l_idx = worker_ptr->last_generated_index.load(std::memory_order_acquire);
                while (l_idx <= g_idx) {
                    worker_ptr->last_generated_index.wait(l_idx, std::memory_order_relaxed);
                    l_idx = worker_ptr->last_generated_index.load(std::memory_order_acquire);
                }
            }

            // Make representation from raw data
            this->representations[wl-1] = std::make_unique<Representation>(wl, std::move(rep_raw_data[g_idx]));

            // Signal to threads to begin generation of next representation
            this->global_index.fetch_add(1, std::memory_order_release);
            this->global_index.notify_all();
            g_idx = this->global_index.load(std::memory_order_relaxed);
        }

        // Finally, gather threads and exit
        this->join_all();
    }


    GroupRepGenerationBundle::~GroupRepGenerationBundle() noexcept {
        this->join_all();
    }

    void GroupRepGenerationBundle::prepare_blank_data() {
        this->rep_raw_data.reserve(this->build_list.size());
        for (auto w : this->build_list) {
            this->rep_raw_data.emplace_back(this->group_size, repmat_t{});
        }
    }

    void GroupRepGenerationBundle::join_all() noexcept {
        for (auto& worker_ptr : this->workers) {
            if (worker_ptr) {
                worker_ptr->join();
            }
        }
    }

}