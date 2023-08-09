/**
 * logging.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "logging.h"

#include "storage_manager.h"

#include "logging/logger.h"
#include "logging/in_memory_logger.h"
#include "logging/to_file_logger.h"

#include "utilities/read_as_string.h"
#include "utilities/read_choice.h"
#include "utilities/reporting.h"

#include <fstream>
#include <sstream>

namespace Moment::mex::functions {

    LoggingParams::LoggingParams(SortedInputs&& raw_input) : SortedInputs(std::move(raw_input)) {

        if (this->inputs.size() >= 1) {
            this->instruction
                    = static_cast<Instruction>(read_choice("First argument",
                                                           {"info", "off", "file", "memory", "output", "clear"},
                                                           inputs[0]));
        } else {
            this->instruction = Instruction::Info;
        };

        switch (this->instruction) {
            case Instruction::SetFile: {
                if (this->inputs.size() != 2) {
                    throw_error(matlabEngine, errors::too_few_inputs, "Log file must be specified.");
                }
                this->filename = this->read_filename(this->inputs[1]);

                break;
            }
            case Instruction::Output:
                if (this->inputs.size() >= 2) {
                    this->filename = this->read_filename(this->inputs[1]);
                }
            default:
                break;
        }
    }

    std::string LoggingParams::read_filename(const matlab::data::Array& input) {
        auto maybe_fn = read_as_utf8(input);
        if (!maybe_fn.has_value()) {
            throw_error(matlabEngine, errors::bad_param, "Log filename must be given as a string.");
        }
        return maybe_fn.value();
    }


    Logging::Logging(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 0;
        this->max_inputs = 2;
        this->min_outputs = 0;
        this->max_outputs = 1;
    }

    void Logging::operator()(IOArgumentRange output, LoggingParams &input) {
        if (output.size() > 0) {
            switch (input.instruction) {
                case LoggingParams::Instruction::Off:
                case LoggingParams::Instruction::SetFile:
                case LoggingParams::Instruction::SetMemory:
                    throw_error(this->matlabEngine, errors::too_many_outputs,
                                "Output only available for info and output subfunctions.");
                default:
                    break;
            }
        }

        switch (input.instruction) {
            case LoggingParams::Instruction::Info:
                this->info(output);
                break;
            case LoggingParams::Instruction::Off:
                this->off();
                break;
            case LoggingParams::Instruction::SetFile:
                this->set_file(input.filename);
                break;
            case LoggingParams::Instruction::SetMemory:
                this->set_memory();
                break;
            case LoggingParams::Instruction::Output:
                if (input.inputs.size() == 2) {
                    this->output_to_file(output, input.filename);
                } else {
                    this->output(output);
                }
                break;
            case LoggingParams::Instruction::Clear:
                this->clear();
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown logging instruction.");
        }
    }

    void Logging::info(IOArgumentRange& output) {
        auto log_ptr = this->storageManager.Logger.get();

        std::stringstream infoSS;
        log_ptr->information(infoSS);
        infoSS << "\n";

        if (output.size() > 0) {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar(infoSS.str());
        }

        if (this->verbose || (output.size() == 0)) {
            print_to_console(this->matlabEngine, infoSS.str());
        }
    }

    void Logging::off() {
        auto ignore_logger = std::make_unique<IgnoreLogger>();
        this->storageManager.Logger.set(std::move(ignore_logger));
    }

    void Logging::set_file(std::string filename) {
        auto file_logger = std::make_unique<ToFileLogger>(std::move(filename));

        this->storageManager.Logger.set(std::move(file_logger));
    }

    void Logging::set_memory() {
        auto memory_logger = std::make_unique<InMemoryLogger>();
        this->storageManager.Logger.set(std::move(memory_logger));
    }

    void Logging::output(IOArgumentRange& output) {
        auto log_ptr = this->storageManager.Logger.get();

        std::stringstream outputSS;
        log_ptr->write_log(outputSS);
        outputSS << "\n";

        if (output.size() > 0) {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar(outputSS.str());
        }

        if (this->verbose || (output.size() == 0)) {
            print_to_console(this->matlabEngine, outputSS.str());
        }
    }

    void Logging::output_to_file(IOArgumentRange& output, const std::string& filename) {
        if (output.size() > 0) {
            throw_error(matlabEngine, errors::too_many_outputs, "No output is returned to matlab if a filename is provided.");
        }

        // Get current logger
        auto log_ptr = this->storageManager.Logger.get();
        if (log_ptr->is_trivial()) {
            throw_error(matlabEngine, errors::bad_param, "Logging is not enabled.");
        }

        // Do write to file
        std::fstream file{filename, std::fstream::out | std::fstream::app };
        log_ptr->write_log(file);
    }

    void Logging::clear() {
        auto log_ptr = this->storageManager.Logger.get();
        log_ptr->clear_log();
    }
}