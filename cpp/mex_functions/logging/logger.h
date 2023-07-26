/**
 * logger.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include <chrono>
#include <iosfwd>
#include <memory>
#include <string>

namespace Moment::mex {

    using LogTime = std::chrono::time_point<std::chrono::system_clock>;
    using LogDuration = std::chrono::duration<double, std::milli>;

    struct LogEvent {
        /** Name of mex function. */
        std::string mex_function;

        /** True if function evaluated without error. */
        bool success;

        /** Number of inputs */
        size_t num_inputs;

        /** Number of outputs. */
        size_t num_outputs;

        /** Time of event. */
        LogTime timestamp;

        /** Time in milliseconds. */
        LogDuration execution_time;

        /** Message, if any */
        std::string additional_info;

        /** Convert to string */
        friend std::ostream& operator<<(std::ostream& os, const LogEvent& event);
    };

    /** Interface for logging. */
    class Logger {
    public:
        virtual ~Logger() = default;

        /**
         * Report an event to the logger.
         */
        virtual void report_event(LogEvent event) = 0;

        /**
         * True if logger does nothing.
         */
        [[nodiscard]]  virtual bool is_trivial() const noexcept { return false; }

        /** Get information about logger itself. */
        virtual void information(std::ostream& os) const = 0;

        /** Get information about logger itself. */
        [[nodiscard]] std::string information_string() const;

        /** Write log, if appropriate */
        virtual void write_log(std::ostream& os) const { }

        /** Clear log, if appropriate */
        virtual void clear_log() { }
    };

    /** Boring 'ignore all' implementation */
    class IgnoreLogger final : public Logger {
    public:
        void report_event(LogEvent event) final { };

        [[nodiscard]] bool is_trivial() const noexcept final { return true; }

        void information(std::ostream& os) const final;
    };

    /**
     * Return current logger singleton.
     */
    [[nodiscard]] std::shared_ptr<Logger> getLogger() noexcept;
}