/**
 * in_memory_logger.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "logger.h"

#include "utilities/maintains_mutex.h"

#include <vector>

namespace Moment::mex {
    class InMemoryLogger : public Logger, private MaintainsMutex {
    private:
        std::vector<LogEvent> log;

    public:
        InMemoryLogger() { }

        void report_event(LogEvent event) override;

        void information(std::ostream& os) const override;

        void write_log(std::ostream &os) const override;

        void clear_log() override;
    };

}