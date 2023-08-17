/**
 * to_file_logger.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "logger.h"

#include "multithreading/queue.h"

#include "multithreading/maintains_mutex.h"

#include <atomic>
#include <deque>
#include <thread>

namespace Moment::mex {
    class ToFileLogger : public Logger, private MaintainsMutex {
    private:
        /** Single-consumer thread for memory log -> file output. */
        class WritingThread {
        private:
            ToFileLogger& logger;
            std::thread thread_object;

            std::atomic_flag has_items_flag;
            std::atomic_flag end_flag;

        public:
            explicit WritingThread(ToFileLogger& logger);

            WritingThread(const WritingThread&) = delete;

            ~WritingThread() noexcept;

            void thread_loop();
        };


    private:
        std::string filename;

        Multithreading::Queue<LogEvent> queue;

        std::optional<WritingThread> file_writer_thread;

    public:
        explicit ToFileLogger(std::string filename);

        ~ToFileLogger() noexcept;

        void report_event(LogEvent event) override;

        void information(std::ostream& os) const override;

        void clear_log() override;

        void write_one_event_to_file(const LogEvent& event);

    private:
        /** Throws exception if cannot write to file. */
        void check_file() const;

    };

}