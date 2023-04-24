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
    };

}