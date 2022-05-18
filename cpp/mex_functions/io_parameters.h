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
#include <string>

namespace NPATK::mex {

    using NameSet = std::set<std::basic_string<char16_t>>;

    using NamedParameter = std::map<std::basic_string<char16_t>, matlab::data::Array>;

    using NamedFlag = NameSet;

    /**
     * Due to matlab's weird usage of templates, matlab::mex::ArgumentList can seemingly only be used in one file
     * without causing linker errors (due to non-inline functions defined in the header file that contains the full
     * definition of MexIORange).
     *
     * Thus, the FlagArgumentRange class implements similar behaviour as matlab::mex::ArgumentList, with the added bonus
     * of compiling and linking when used across more than one compilation unit.
     */
    class FlagArgumentRange {
    public:
        using iter_type = std::vector<matlab::data::Array>::iterator;

    private:
        iter_type i_start, i_end;
        size_t elem_count;

    public:
        constexpr FlagArgumentRange(iter_type first, iter_type end)
                : i_start(first), i_end(end), elem_count{static_cast<size_t>(std::distance(first, end))} {}

        FlagArgumentRange(const FlagArgumentRange &rhs) = default;

        [[nodiscard]] constexpr size_t size() const { return elem_count; }

        [[nodiscard]] constexpr iter_type begin() { return i_start; }

        [[nodiscard]] constexpr iter_type end() { return i_end; }

        /**
         * Get first element in list, and remove from list.
         * @return
         */
        matlab::data::Array& pop_front() {
            assert(elem_count > 0);
            auto& out = *(this->i_start);
            ++this->i_start;
            --elem_count;
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
        NamedParameter params;
        NamedFlag flags;
        std::vector<matlab::data::Array> inputs;


        SortedInputs() = default;
        SortedInputs(const SortedInputs& rhs) = delete;
        SortedInputs(SortedInputs&& rhs) = default;
    };

}