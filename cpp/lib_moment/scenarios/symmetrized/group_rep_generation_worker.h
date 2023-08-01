/**
 * group_rep_generation_worker.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading/multithreading.h"
#include "group.h"

#include <atomic>
#include <thread>

namespace Moment::Multithreading {
    class GroupRepGenerationBundle;

    class GroupRepGenerationWorker {
    public:
        GroupRepGenerationBundle& bundle;
        const size_t worker_id;
        const size_t max_workers;

    private:
        std::atomic<size_t> last_generated_index;
        std::thread the_thread;

    public:
        GroupRepGenerationWorker(GroupRepGenerationBundle& bundle,
                                 size_t worker_id, size_t max_workers);

    public:
        void join() noexcept;

        void launch();

        void execute();

        friend class GroupRepGenerationBundle;
    };

    class GroupRepGenerationBundle {

    public:
        const size_t max_workers;
        const size_t group_size;
        const Symmetrized::Group::build_list_t build_list;

    private:
        std::vector<std::unique_ptr<Symmetrized::Representation>>& representations;

        std::vector<std::unique_ptr<Symmetrized::RepresentationMapper>>& mappers;

        std::vector<std::unique_ptr<GroupRepGenerationWorker>> workers;

        std::vector<std::vector<Symmetrized::repmat_t>> rep_raw_data;

        std::atomic<size_t> global_index;

    public:
        GroupRepGenerationBundle(std::vector<std::unique_ptr<Symmetrized::Representation>>& representations,
                                 std::vector<std::unique_ptr<Symmetrized::RepresentationMapper>>& mappers,
                                 const size_t group_size,
                                 Symmetrized::Group::build_list_t build_list);

        ~GroupRepGenerationBundle() noexcept;

        void execute();

        friend class GroupRepGenerationWorker;

    private:
        void prepare_blank_data();

        void join_all() noexcept;

    };

}