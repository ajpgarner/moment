/**
 * logging.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mex_function.h"

#include <string>

namespace Moment::mex::functions {

    struct LoggingParams : public SortedInputs {
    public:
        explicit LoggingParams(SortedInputs&& inputs);

        enum class Instruction {
            Info,
            Off,
            SetFile,
            SetMemory,
            Output,
            Clear,
        } instruction = Instruction::Info;

        std::string filename;

    private:
        [[nodiscard]] std::string read_filename(const matlab::data::Array& input);
    };

    class Logging : public ParameterizedMexFunction<LoggingParams, MEXEntryPointID::Logging> {
    public:
        explicit Logging(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, LoggingParams &input) override;

    private:
        void info(IOArgumentRange& output);

        void off();

        void set_file(std::string filename);

        void set_memory();

        void output(IOArgumentRange& output);

        void output_to_file(IOArgumentRange& output, const std::string& filename);

        void clear();

    };
}