/**
 * io_parameters.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 */
#pragma once

#include "MatlabDataArray.hpp"

#include <cassert>
#include <set>
#include <map>
#include <optional>
#include <string>

#include "error_codes.h"

namespace NPATK::mex {

    using ParamNameStr = std::basic_string<char16_t>;

    using NameSet = std::set<ParamNameStr>;

    using NamedParameter = std::map<ParamNameStr, matlab::data::Array>;

    using NamedFlag = NameSet;


    class MutuallyExclusiveParams {
        std::multimap<ParamNameStr, ParamNameStr> pairs;
    public:
        /**
         * Register two flags/parameters as mutually exclusive
         * @param str_a The first flag/parameter name
         * @param str_b The second flag/parameter name
         */
        void add_mutex(const ParamNameStr& str_a, const ParamNameStr& str_b);

        /**
         * Detects if a set of flags and parameters violates any mutual exclusions.
         * @param flags The set of flags
         * @param params The set of named parameters
         * @return Violating pair of names, or empty std::optional if no violations found.
         */
        [[nodiscard]] std::optional<std::pair<ParamNameStr, ParamNameStr>> validate(const NameSet& flags,
                                                                                    const NamedParameter& params) const;
    };

    /**
     * Due to matlab's weird usage of templates, matlab::mex::ArgumentList can seemingly only be used in one file
     * without causing linker errors (due to non-inline functions defined in the header file that contains the full
     * definition of MexIORange).
     *
     * Thus, the IOArgumentRange class implements similar behaviour as matlab::mex::ArgumentList, with the added bonus
     * of compiling and linking when used across more than one compilation unit.
     */
    class IOArgumentRange {
    public:
        using iter_type = std::vector<matlab::data::Array>::iterator;

    private:
        iter_type i_start, i_end;
        size_t elem_count;

    public:
        constexpr IOArgumentRange(iter_type first, iter_type end)
                : i_start(first), i_end(end), elem_count{static_cast<size_t>(std::distance(first, end))} {}

        IOArgumentRange(const IOArgumentRange &rhs) = default;

        [[nodiscard]] constexpr size_t size() const { return elem_count; }

        [[nodiscard]] constexpr iter_type begin() { return i_start; }

        [[nodiscard]] constexpr iter_type end() { return i_end; }

        /**
         * Get first element in list, and remove from list.
         */
        matlab::data::Array& pop_front() {
            assert(elem_count > 0);
            auto& out = *(this->i_start);
            ++this->i_start;
            --elem_count;
            // It is safe to return a reference because the range is non-owning!
            return out;
        }

        constexpr matlab::data::Array &operator[](iter_type::difference_type elem) {
            assert((elem >= 0) && (elem < this->elem_count));
            return *(i_start + elem);
        }
    };

    namespace errors {
        struct BadInput : std::runtime_error {
        public:
            std::string errCode;

            BadInput(std::string errCode, const std::string& what)
                    : std::runtime_error{what}, errCode{std::move(errCode)} { }
        };
    }

    /**
     * Pre-processed inputs to functions
     */
    struct SortedInputs {
    public:
        NamedParameter params;
        NamedFlag flags;
        std::vector<matlab::data::Array> inputs;

        SortedInputs() = default;
        virtual ~SortedInputs() = default;

        SortedInputs(const SortedInputs& rhs) = delete;
        SortedInputs(SortedInputs&& rhs) = default;

        matlab::data::Array& find_or_throw(const ParamNameStr& paramName);

        [[nodiscard]] virtual std::string to_string() const;

    public:
        /**
         * Read integer, or throw BadInput exception.
         * @param matlabEngine Reference to engine.
         * @param paramName Parameter/input name, as will appear in failure error message.
         * @param array The array to attempt to parse as a scalar integer.
         * @param min_value The minimum acceptable value of the integer.
         * @return The parsed integer.
         */
        static uint64_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
                                          const std::string& paramName, const matlab::data::Array& array,
                                          uint64_t  min_value = 0);

        /**
         * Read integer, or throw BadInput exception.
         * @param matlabEngine Reference to engine.
         * @param paramName Parameter/input name, as will appear in failure error message.
         * @param array The array to attempt to parse as a scalar integer.
         * @param min_value The minimum acceptable value of the integer.
         * @return The parsed integer.
         */
        static uint64_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
                                          const std::string& paramName, const matlab::data::MATLABString& mlString,
                                          uint64_t  min_value = 0);
    };

}