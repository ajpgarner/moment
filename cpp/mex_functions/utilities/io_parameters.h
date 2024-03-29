/**
 * io_parameters.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 *
 */
#pragma once

#include "MatlabDataArray.hpp"

#include <cassert>
#include <set>
#include <map>
#include <optional>
#include <string>

#include "errors.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

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
         * Register a set of parameters as mutually exclusive
         */
        void add_mutex(std::initializer_list<ParamNameStr>&& list);

        /**
         * Register a set of parameters as mutually exclusive
         */
        void add_mutex(const NameSet& mutex_list);

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


    /**
     * Pre-processed inputs to functions
     */
    struct SortedInputs {
    protected:
        matlab::engine::MATLABEngine& matlabEngine;

    public:
        NamedParameter params;
        NamedFlag flags;
        std::vector<matlab::data::Array> inputs;

        explicit SortedInputs(matlab::engine::MATLABEngine& engine) : matlabEngine{engine} { }
        SortedInputs(const SortedInputs& rhs) = delete;
        SortedInputs(SortedInputs&& rhs) = default;

        virtual ~SortedInputs() = default;



        [[nodiscard]] std::optional<size_t> get_index_of_matched_flag(const NameSet& matches) const;

        [[nodiscard]] virtual std::string to_string() const;

        /** Get named parameter, or throw an error */
        [[nodiscard]] matlab::data::Array& find_or_throw(const ParamNameStr& paramName);

        /**
         * Execute provided functor if named parameter is set.
         * @return True, if parameter was found.
         */
        template<typename functor_t>
        bool find_and_parse(const ParamNameStr& paramName, functor_t action) {
            auto find_iter = this->params.find(paramName);
            if (find_iter == this->params.cend()) {
                return false;
            }
            action(find_iter->second);
            return true;
        }

        /** Execute provided functor if named parameter is set; otherwise throw an error. */
        template<typename functor_t>
        void find_and_parse_or_throw(const ParamNameStr& paramName, functor_t action) {
            auto& found_param = this->find_or_throw(paramName);
            action(found_param);
        }

    };

    /** Error code: thrown when a named parameter should be present, but is not. */
    class MissingParamException : public MomentMEXException {
    public:
        const std::string missing_parameter;

        explicit MissingParamException(const std::string& missing_name)
            : MomentMEXException{"missing_param", make_msg( missing_name)}, missing_parameter{missing_name} { }

        [[nodiscard]] static std::string make_msg(const std::string& missing_name);
    };

}