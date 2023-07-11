/**
 * exporter.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "MatlabDataArray.hpp"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    class Exporter {
    public:
        matlab::engine::MATLABEngine& engine;
        matlab::data::ArrayFactory& factory;

        explicit Exporter(matlab::engine::MATLABEngine& engine,
                          matlab::data::ArrayFactory& factory) noexcept : engine{engine}, factory{factory} { }

    public:
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

    private:
        [[noreturn]] void report_too_small_output() const;

        [[noreturn]] void report_too_small_input() const;

    };

    class ExporterWithFactory : public Exporter {
    private:
        mutable matlab::data::ArrayFactory own_factory{};

    public:
        explicit ExporterWithFactory(matlab::engine::MATLABEngine& engine)
            : Exporter{engine, own_factory} { }

    };


}