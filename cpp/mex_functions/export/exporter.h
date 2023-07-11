/**
 * exporter.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    class Exporter {
    protected:
        matlab::engine::MATLABEngine& engine;
    public:
        explicit Exporter(matlab::engine::MATLABEngine& engine) noexcept : engine{engine} { }

    protected:
        template<typename read_iter_t, typename write_iter_t, typename functor_t>
        void do_write(read_iter_t read_iter, const read_iter_t read_iter_end,
                      write_iter_t write_iter, const write_iter_t write_iter_end,
                      const functor_t& functor) const {
            while ((read_iter != read_iter_end) && (write_iter != write_iter_end)) {
                *write_iter = functor(*read_iter);
                ++read_iter;
                ++write_iter;
            }
            if (read_iter != read_iter_end) {
                report_too_small_output();
            }
            if (write_iter != write_iter_end) {
                report_too_small_input();
            }
        }

    protected:
        [[noreturn]] void report_too_small_output() const;

        [[noreturn]] void report_too_small_input() const;

    };

}