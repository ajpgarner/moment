/**
 * errors.h
 * 
 * @copyright Copyright (c) 2022-2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <stdexcept>
#include <string>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    /** Error thrown during the evaluation of a MEX function. */
    class MomentMEXException : public std::exception {
        /** Prefix to all error identifier codes. */
        constexpr static char prefix[] = "mtk:";

    public:
        /** Error identifier */
        const std::string error_code;

        /**
         * Exception, to be ultimately passed to MATLAB.
         * @param code Error identifier (without prefix).
         * @param what Error message.
         */
        MomentMEXException(const std::string& code, const std::string& what)
            : std::exception{what.c_str()}, error_code{apply_prefix(code)} { }

        /** Rethrow exception as MATLAB error, to be handled within MATLAB. */
        [[noreturn]] void throw_to_MATLAB(matlab::engine::MATLABEngine& engine) const;

        /** Applies prefix to error code */
        [[nodiscard]] constexpr static std::string apply_prefix(const std::string& errCode) {
            return std::string(prefix) + errCode;
        }

    };

    /** Error code: thrown when logical assertion fails, and it's not the users (direct) fault. */
    class InternalError : public MomentMEXException {
    public:
        explicit InternalError(const std::string& what)
            : MomentMEXException{"internal_error", what} {};
    };


    /** Error code: thrown when function name does not exist. */
    class BadFunctionException : public MomentMEXException {
    public:
        /** Error code: thrown when requested function cannot be determined. */
        constexpr static char bad_function[] = "bad_function";

        /** Bad function: unreadable function name. */
        BadFunctionException();

        /** Bad function: missing function name. */
        explicit BadFunctionException(const std::string& func_name);
    };

    /** Exception thrown when input count is wrong. */
    class InputCountException : public MomentMEXException {
    public:
        /** Error code: thrown when inputs are missing */
        constexpr static char too_few_inputs[] = "too_few_inputs";

        /** Error code: thrown when there are too many inputs */
        constexpr static char too_many_inputs[] = "too_many_inputs";

        /** Function where mismatch occurred. */
        const std::string function_name;

        /** Lower bound on inputs */
        const size_t min_expected;

        /** Upper bound on inputs */
        const size_t max_expected;

        /** Actual number of inputs */
        const size_t actual;

        /** Automatically formatted input count error. */
        InputCountException(const std::string& func_name, size_t min_expected, size_t max_expected, size_t actual);

        /** Manually formatted input count error. */
        InputCountException(const std::string& func_name, size_t min_expected, size_t max_expected, size_t actual,
                            const std::string& what);
    };

    /** Exception thrown when output count is wrong. */
    class OutputCountException : public MomentMEXException {
    public:

        /** Error code: thrown when outputs are missing */
        constexpr static char too_few_outputs[] = "too_few_outputs";

        /** Error code: thrown when there are too many outputs */
        constexpr static char too_many_outputs[] = "too_many_outputs";

        /** Function where mismatch occurred. */
        const std::string function_name;

        /** Lower bound on outputs */
        const size_t min_expected;

        /** Upper bound on outputs */
        const size_t max_expected;

        /** Actual number of outputs */
        const size_t actual;

        /** Automatically formatted output count error. */
        OutputCountException(const std::string& func_name, size_t min_expected, size_t max_expected, size_t actual);

        /** Manually formatted output count error. */
        OutputCountException(const std::string& func_name, size_t min_expected, size_t max_expected, size_t actual,
                             const std::string& what);
    };

    /** Exception thrown when two (or more) mutually exclusive flags/parameters are provided. */
    class MutexParamException : public MomentMEXException {
    public:
        const std::string func_name;
        const std::string param1;
        const std::string param2;

        MutexParamException(const std::string& func_name, const std::string& param1, const std::string& param2);
    };

    /** Generic exception thrown when the user has provided bad input. */
    class BadParameter : public MomentMEXException {
    public:
        explicit BadParameter(const std::string& what) : MomentMEXException{"bad_param", what} {};
    };

    /** Internal error caused due to bad dynamic_cast. */
    class BadCastException : public MomentMEXException {
    public:
        explicit BadCastException(const std::string& what) : MomentMEXException{"bad_cast", what} {};
    };

    /**
     * Internal error caused when visitor-pattern fails.
     */
    class BadVisitorException : public MomentMEXException {
    public:
        explicit BadVisitorException(const std::string& what) : MomentMEXException{"bad_visit", what} { }
    };

    /**
     * Error caused when something is wrong with the persistent storage.
     */
    class StorageManagerError : public MomentMEXException {
    public:
        explicit StorageManagerError(const std::string& what) : MomentMEXException{"storage_error", what} { }
    };

    /**
     * Failure encountered with symbolization.
     */
    class BadSymbol : public MomentMEXException {
    public:
        explicit BadSymbol(const std::string& what) : MomentMEXException{"bad_symbol", what} { }
    };

}
