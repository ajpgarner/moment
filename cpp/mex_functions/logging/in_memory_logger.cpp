/**
 * in_memory_logger.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "in_memory_logger.h"

namespace Moment::mex {

    void InMemoryLogger::report_event(LogEvent event) {
        auto lock = this->get_write_lock();
        this->log.emplace_back(std::move(event));
    }

    void InMemoryLogger::information(std::ostream &os) const {
        os << "Logging to memory.";
    }

    void InMemoryLogger::write_log(std::ostream &os) const {
        auto lock = this->get_read_lock();
        for (const auto& entry : this->log) {
            os << entry;
        }
    }

    void InMemoryLogger::clear_log() {
        auto lock = this->get_write_lock();
        this->log.clear();
    }


}