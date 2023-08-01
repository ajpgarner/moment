/**
 * logger.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "logger.h"

#ifdef __cpp_lib_format
#include <format>
#else
#include <ctime>
#include <iomanip>
#endif

#include <iostream>
#include <sstream>

namespace Moment::mex {

    std::ostream& operator<<(std::ostream& os, const LogEvent& event) {
#ifdef __cpp_lib_format
        os << std::format("{0:%F}T{0:%R:%OS%z}", event.timestamp);
#else
        const time_t time_int = std::chrono::system_clock::to_time_t(event.timestamp);
        os << std::put_time(std::localtime(&time_int), "%FT%T%z");
#endif
        os << "\t`" << event.mex_function << "` ";

        if (event.success) {
            os << "succeeded in ";
        } else {
            os << "failed after ";
        }
        os << event.execution_time.count() << " ms.\t";

        os << event.num_inputs << "/" << event.num_outputs << "\t";
        if (!event.additional_info.empty()) {
            os << "\t" << event.additional_info;
        }
        os << std::endl;
        return os;
    }

    std::string Logger::information_string() const {
        std::stringstream ss;
        this->information(ss);
        return ss.str();
    }

    void IgnoreLogger::information(std::ostream &os) const {
        os << "Logging disabled.";
    }

}
