/**
 * to_file_logger.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "to_file_logger.h"

#include <fstream>
#include <iostream>

namespace Moment::mex {

    ToFileLogger::ToFileLogger(std::string filename) : filename(std::move(filename)) {
        try {
            auto lock = this->get_write_lock();
            std::fstream os{this->filename, std::fstream::out | std::fstream::app};
            os.close();
        }
        catch(std::exception& e) {
            std::stringstream errSS;
            errSS << "Cannot write to log file \"" << this->filename << "\".";
            throw std::runtime_error{errSS.str()};
        }
    }

    void ToFileLogger::report_event(Moment::mex::LogEvent event) {
        auto lock = this->get_write_lock();
        try {
            std::fstream os{this->filename, std::fstream::out | std::fstream::app};
            os << event;
            os.close();
        } catch(std::exception& e) {
            std::stringstream errSS;
            errSS << "Cannot write to log file \"" << this->filename << "\".";
            throw std::runtime_error{errSS.str()};
        }
        // ~lock
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