/**
 * mex_main.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "mex_main.h"

#include "logging/in_memory_logger.h"

#include "utilities/reporting.h"
#include "utilities/read_as_string.h"

#include "function_list.h"
#include "mtk_function.h"

#include "utilities/utf_conversion.h"

#include "storage_manager.h"

#include <chrono>
#include <sstream>

namespace Moment::mex {

    namespace {

        /** RAII Log wrapper */
        class LogTrigger {
        private:
            Logger &logger;
            std::optional<LogEvent> event;
            std::chrono::time_point<std::chrono::high_resolution_clock> precision_start;

        public:

            LogTrigger(Logger &logger,
                       functions::MTKEntryPointID function_id,
                       size_t num_in, size_t num_out)
                    : logger{logger} {
                if (logger.is_trivial()) {
                    return;
                }

                precision_start = std::chrono::high_resolution_clock::now();
                event.emplace(functions::which_function_name(function_id),
                              num_in, num_out,
                              std::chrono::system_clock::now());
            }

            ~LogTrigger()  {
                try {
                    this->reset();
                } catch (...) {
                    // Ignore all exceptions thrown by logger.
                }
            }

            void reset() noexcept {
                if (this->event.has_value()) {
                    this->end_timer();
                    logger.report_event(std::move(this->event.value()));
                    this->event.reset();
                }
            }

            void end_timer() {
                auto precision_end = std::chrono::high_resolution_clock::now();
                this->event->execution_time = precision_end - precision_start;
            }

            void report_success() noexcept {
                this->event->success = true;
            }

            void report_failure(std::string reason = "") noexcept {
                this->event->success = false;
                this->event->additional_info = std::move(reason);
            }
        };
    }


    MexMain::MexMain(std::shared_ptr <matlab::engine::MATLABEngine> matlab_engine, StorageManager& storage)
        : matlabPtr(std::move(matlab_engine)), persistentStorage{storage} {
        this->logger = persistentStorage.Logger.get();
    }

    MexMain::~MexMain() noexcept = default;

    void MexMain::operator()(IOArgumentRange outputs, IOArgumentRange inputs) {
        // Start timer

        // Read and pop function name...
        functions::MTKEntryPointID function_id = get_function_id(inputs);
        assert(function_id != functions::MTKEntryPointID::Unknown);

        auto log_entry = LogTrigger{*this->logger,
                                    function_id, inputs.size(), outputs.size()};

        // Execute function
        try {

            // Construction function object from ID...
            std::unique_ptr<functions::MTKFunction> the_function = functions::make_mtk_function(*matlabPtr,
                                                                                                function_id,
                                                                                                persistentStorage);
            if (!the_function) {
                throw_error(*matlabPtr, errors::bad_function, u"Internal error: could not create function object.");
            }

            // Get named parameters & flags
            auto processed_inputs = this->clean_inputs(*the_function, inputs);
            assert(processed_inputs);

            // Check inputs are in range, and are valid
            this->validate_inputs(*the_function, *processed_inputs);

            // Pre-process universal input flags
            bool is_debug = processed_inputs->flags.contains(u"debug");
            bool is_verbose = is_debug || processed_inputs->flags.contains(u"verbose");
            bool is_quiet = processed_inputs->flags.contains(u"quiet") && !is_verbose;

            bool preprocess_only = processed_inputs->flags.contains(u"debug_preprocess");

            the_function->setQuiet(is_quiet);
            the_function->setDebug(is_debug);
            the_function->setVerbose(is_verbose);

            // Final function-specific pre-processing and validation of inputs (transfer ownership to function)
            processed_inputs = this->transform_and_validate(*the_function, std::move(processed_inputs), outputs);

            // Check outputs are in range
            this->validate_outputs(*the_function, outputs, *processed_inputs);

            // If only transforming parameters, print output:
            if (preprocess_only) {
                print_to_console(*this->matlabPtr, processed_inputs->to_string());
                matlab::data::ArrayFactory factory{};
                for (auto &output: outputs) {
                    output = factory.createScalar(0);
                }
                return;
            }

            (*the_function)(outputs, std::move(processed_inputs));
            log_entry.report_success();
        } catch (std::exception& e) {
            log_entry.report_failure(e.what());
            throw;
        }

        // ~the_function, ~log_entry
    }

    functions::MTKEntryPointID MexMain::get_function_id(IOArgumentRange& inputs) {
        if (inputs.size() <= 0) {
            return functions::MTKEntryPointID::Version;
        }

        auto command_arg = read_as_utf8(inputs.pop_front());
        if (!command_arg.has_value()) {
            throw_error(*matlabPtr, errors::bad_function,
                        "First argument must be a single function name (i.e. one string).");
        }

        auto entry_id = functions::which_entrypoint(command_arg.value());
        if (entry_id == functions::MTKEntryPointID::Unknown) {
            throw_error(*matlabPtr, errors::bad_function,
                        "Function \"" + command_arg.value() + "\" is not in the Moment library.");
        }

        return entry_id;
    }


    std::unique_ptr<SortedInputs> MexMain::clean_inputs(const functions::MTKFunction &func, IOArgumentRange & inputs) {
        const auto& param_names = func.ParamNames();
        const auto& func_flag_names = func.FlagNames();

        // Incorporate default flags:~
        NameSet flag_names = {u"quiet", u"verbose", u"debug", u"debug_preprocess"};
        flag_names.insert(func_flag_names.begin(), func_flag_names.end());

        std::unique_ptr<SortedInputs> sortedPtr = std::make_unique<SortedInputs>(*matlabPtr);
        assert(sortedPtr);
        SortedInputs& sorted = *sortedPtr;

        // Scan through inputs
        size_t cursor = 0;
        auto input_iter = inputs.begin();
        while (input_iter != inputs.end()) {
            auto& input = *input_iter;
            bool matched = false;

            auto maybe_string = read_as_utf16(input);
            if (maybe_string.has_value()) {
                const auto& param_flag_str = maybe_string.value();

                // First, is input a parameter?
                if (param_names.contains(param_flag_str)) {
                    if ((cursor+1) >= inputs.size()) {
                        throw_error(*this->matlabPtr, errors::bad_param,
                                                      u"Named parameter \"" + param_flag_str + u"\" was used,"
                                                      + u" but next argument (with data) is missing.");
                    } else {
                        ++input_iter;
                        auto& data = *input_iter;
                        sorted.params.emplace(param_flag_str, std::move(data));
                        matched = true;
                    }
                } else if (flag_names.contains(param_flag_str)) {
                    sorted.flags.emplace(param_flag_str);
                    matched = true;
                }
            }

            // Otherwise, input is flat, push to back of list...
            if (!matched) {
                sorted.inputs.emplace_back(std::move(input));
            }

            // Advance iterators...
            ++input_iter;
            ++cursor;
        }

        return sortedPtr;
    }

    void MexMain::validate_inputs(const functions::MTKFunction &func, const SortedInputs &inputs) {

        // First check number of inputs is okay
        auto [min, max] = func.NumInputs();
        if ((inputs.inputs.size() > max) || (inputs.inputs.size() < min)) {
            // Build error message:
            std::stringstream ss;
            ss << "Function \"" << functions::which_function_name(func.function_id) << "\" ";
            if (min != max) {
                ss << "requires between " << min << " and " << max << " input parameters.";
            } else {
                if (min == 0) {
                    ss << "does not take an input.";
                } else {
                    ss << "requires " << min;
                    if (min != 1) {
                        ss << " input parameters.";
                    } else {
                        ss << " input parameter.";
                    }
                }
            }

            if (inputs.inputs.size() > max) {
                throw_error(*matlabPtr, errors::too_many_inputs, ss.str());
            } else {
                throw_error(*matlabPtr, errors::too_few_inputs, ss.str());
            }
        }

        // Next, check for mutual exclusion
        auto mutex_params = func.check_for_mutex(inputs);
        if (mutex_params.has_value()) {
            UTF16toUTF8Convertor convertor;
            std::stringstream bss;
            bss << "Invalid argument to function \"" << functions::which_function_name(func.function_id) << "\": "
                << "Cannot specify mutually exclusive parameters \"" << convertor(mutex_params->first) << "\""
                << " and \"" << convertor(mutex_params->second) << "\".";
            throw_error(*matlabPtr, errors::mutex_param, bss.str());
        }

    }

    void MexMain::validate_outputs(const functions::MTKFunction &func,
                                   const IOArgumentRange &outputs,  const SortedInputs &inputs) {
        auto [min, max] = func.NumOutputs();
        if ((outputs.size() > max) || (outputs.size() < min)) {

            // Build error message:
            std::stringstream ss;
            ss << "Function \"" << functions::which_function_name(func.function_id) << "\" ";
            if (min != max) {
                ss << "requires between " << min << " and " << max << " outputs.";
            } else {
                if (min == 0) {
                    ss << "does not have an output.";
                } else {
                    ss << "requires " << min;
                    if (min != 1) {
                        ss << " outputs.";
                    } else {
                        ss << " output.";
                    }
                }
            }

            if (outputs.size() > max) {
                throw_error(*matlabPtr, errors::too_many_outputs, ss.str());
            } else {
                throw_error(*matlabPtr, errors::too_few_outputs, ss.str());
            }
        }

        // Function-specific validation
        func.validate_output_count(outputs.size(), inputs);
    }

    std::unique_ptr<SortedInputs> MexMain::transform_and_validate(const functions::MTKFunction& func,
                                                         std::unique_ptr<SortedInputs> inputs,
                                                         const IOArgumentRange& outputs) {
        try {
            // Call function's own custom validator
            return func.transform_inputs(std::move(inputs));

        } catch (const errors::BadInput& bie) {
            std::stringstream errSS;
            errSS << "Invalid argument to function \"" << functions::which_function_name(func.function_id) << "\": "
                << bie.what();
            throw_error(*matlabPtr, bie.errCode, errSS.str());
        }
    }



}