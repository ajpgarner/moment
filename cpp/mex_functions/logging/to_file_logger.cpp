/**
 * to_file_logger.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "to_file_logger.h"

#include <fstream>
#include <iostream>
#include <thread>

namespace Moment::mex {

    ToFileLogger::WritingThread::WritingThread(ToFileLogger &logger)
        : logger{logger} {

        // Begin thread
        this->thread_object = std::thread(&WritingThread::thread_loop, this);

    }

    ToFileLogger::WritingThread::~WritingThread() noexcept {
        if (this->thread_object.joinable()) {
            // If throws exception in noexcept, failure to join was going to std::terminate anyway, so let it abort.
            this->thread_object.join();
        }
    }

    void ToFileLogger::WritingThread::thread_loop() {
        while (true) {

            // Get item
            auto node_ptr = this->logger.queue.wait_pop_front();

            // If no item, check if queue has been shut down; otherwise reset and wait again
            if (node_ptr == nullptr) {
                if (this->logger.queue.aborting()) {
                    return;
                } else {
                    continue;
                }
            }
            auto& event = **node_ptr;

            try {
                auto file_lock = this->logger.get_write_lock();
                std::fstream os{this->logger.filename, std::fstream::out | std::fstream::app};
                os << event;
                // ~file_lock
            } catch(std::exception& e) {
                std::stringstream errSS;
                errSS << "Cannot write to log file \"" << this->logger.filename << "\": " << e.what();
                throw std::runtime_error{errSS.str()}; // Could just abort() as not in main thread...!
            }
        }
    }


    ToFileLogger::ToFileLogger(std::string filename) : filename(std::move(filename)) {
        // First, test file works.
        try {
            std::fstream os{this->filename, std::fstream::out | std::fstream::app};
        }
        catch(std::exception& e) {
            std::stringstream errSS;
            errSS << "Cannot write to log file \"" << this->filename << "\": " << e.what();
            throw std::runtime_error{errSS.str()};
        }

        // Next, launch writer thread
        this->file_writer_thread.emplace(*this);
    }

    ToFileLogger::~ToFileLogger() noexcept {
        this->queue.abort();
        this->file_writer_thread.reset();
    }

    void ToFileLogger::report_event(Moment::mex::LogEvent event) {
        this->queue.push_back(std::move(event));
    }

    void ToFileLogger::information(std::ostream &os) const {
        os << "Logging to file \"" << this->filename << "\".";
    }

    void ToFileLogger::clear_log() {
        auto lock = this->get_write_lock();
        try {
            std::fstream os{this->filename, std::fstream::out | std::fstream::trunc};
            os.close();
        } catch(std::exception& e) {
            std::stringstream errSS;
            errSS << "Cannot clear log file \"" << this->filename << "\".";
            throw std::runtime_error{errSS.str()};
        }
    }
}