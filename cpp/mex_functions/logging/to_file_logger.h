/**
 * to_file_logger.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "logger.h"

#include "utilities/maintains_mutex.h"

namespace Moment::mex {
    class ToFileLogger : public Logger, private MaintainsMutex {
    private:
        std::string filename;

    public:
        explicit ToFileLogger(std::string filename);

        void report_event(LogEvent event) override;

        void information(std::ostream& os) const override;

        void clear_log() override;

    };

}