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
#include <stdexcept>
#include <string>

namespace Moment::mex {

    namespace {

        /** RAII Log wrapper */
        class LogTrigger {
        private:
            Logger& logger;
            std::optional<LogEvent> event;
            std::chrono::time_point<std::chrono::high_resolution_clock> precision_start;

        public:

            LogTrigger(Logger& logger,
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

            ~LogTrigger() {
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
                throw InternalError{"Internal error: could not create function object."};
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
        } catch (const MomentMEXException& me) { // Errors that we expect to pass to MATLAB.
            log_entry.report_failure(me.what());

            // Rethrow (using MATLAB engine)
            me.throw_to_MATLAB(*this->matlabPtr);
        } catch (const std::exception& e) { // Errors that we don't expect to pass to MATLAB, but will pass anyway.
            log_entry.report_failure(e.what());

            // Convert error to MATLAB's tagged format
            std::stringstream errWhat;
            errWhat << "An unhandled C++ exception was encountered: " << e.what();
            InternalError err{errWhat.str()};

            // Rethrow (using MATLAB engine)
            err.throw_to_MATLAB(*this->matlabPtr);
        }

        // ~the_function, ~log_entry
    }

    functions::MTKEntryPointID MexMain::get_function_id(IOArgumentRange& inputs) {
        if (inputs.size() <= 0) {
            return functions::MTKEntryPointID::Version;
        }

        auto command_arg = read_as_utf8(inputs.pop_front());

        // Error if we cannot read function name:
        if (!command_arg.has_value()) {
            throw BadFunctionException{};
        }

        auto entry_id = functions::which_entrypoint(command_arg.value());
        // Error if we cannot find function matching name:
        if (entry_id == functions::MTKEntryPointID::Unknown) {
            throw BadFunctionException{command_arg.value()};
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
                        UTF16toUTF8Convertor convertor{};
                        std::stringstream errSS;
                        errSS << "Named parameter \"" << UTF16toUTF8Convertor::convert_as_ascii(param_flag_str) << "\""
                              << " was used, but next argument (with data) is missing.";
                        throw BadParameter{errSS.str()};
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
            throw InputCountException{functions::which_function_name(func.function_id),
                                      min, max, inputs.inputs.size()};
        }

        // Next, check for mutual exclusion
        auto mutex_params = func.check_for_mutex(inputs);
        if (mutex_params.has_value()) {
            UTF16toUTF8Convertor convertor;
            throw MutexParamException{functions::which_function_name(func.function_id),
                                      convertor(mutex_params->first), convertor(mutex_params->second)};
        }

    }

    void MexMain::validate_outputs(const functions::MTKFunction &func,
                                   const IOArgumentRange &outputs,  const SortedInputs &inputs) {
        auto [min, max] = func.NumOutputs();
        if ((outputs.size() > max) || (outputs.size() < min)) {
            throw OutputCountException{functions::which_function_name(func.function_id) ,
                                       min, max, outputs.size()};
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
        } catch (const BadParameter& bpe) {
            // Throw new version of exception, tagged with function name prefix:
            std::stringstream errSS;
            errSS << "Invalid argument to function \"" << functions::which_function_name(func.function_id) << "\": "
                  << bpe.what();
            throw BadParameter{errSS.str()};
        }
    }



}