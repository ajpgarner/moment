/**
 * io_parameters.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "utilities/reflection.h"

#include "utilities/utf_conversion.h"

#include "io_parameters.h"

#include "mex.hpp"

#include <algorithm>
#include <memory>

namespace Moment::mex {

    void MutuallyExclusiveParams::add_mutex(const ParamNameStr &str_a, const ParamNameStr &str_b) {
        if (str_a < str_b) {
            this->pairs.emplace(str_a, str_b);
        } else {
            this->pairs.emplace(str_b, str_a);
        }
    }

    std::optional<std::pair<ParamNameStr, ParamNameStr>>
    MutuallyExclusiveParams::validate(const NameSet &flags, const NamedParameter &params) const  {

        // Copy parameter names:
        std::set<ParamNameStr> param_names{};
        std::transform(params.cbegin() , params.cend(), std::inserter(param_names, param_names.begin()),
                       [](const auto& pair) { return pair.first; });

        // Iterate through flags first
        auto flag_iter = flags.cbegin();
        while (flag_iter != flags.cend()) {
            const auto& flag = *flag_iter;
            // Find if flag has any mutual exclusions
            auto [mutex_start, mutex_end] = this->pairs.equal_range(flag);

            // Skip if not....
            if (std::distance(mutex_start, mutex_end) <= 0) {
                ++flag_iter;
                continue;
            }
            // Names that could clash:
            std::set<ParamNameStr> excluded_names{};
            std::transform(mutex_start , mutex_end, std::inserter(excluded_names, excluded_names.begin()),
                           [](const auto& pair) { return pair.second; });

            // Check for clashes with flags
            std::set<ParamNameStr> clashes;
            std::set_intersection(excluded_names.cbegin(), excluded_names.cend(),
                                  flags.begin(), flags.cend(),
                                  std::inserter(clashes, clashes.begin()));

            // Check for clashes with parameter names
            std::set_intersection(excluded_names.cbegin(), excluded_names.cend(),
                                  param_names.cbegin(), param_names.cend(),
                                  std::inserter(clashes, clashes.end()));

            // If a clash, return matching flags
            if (!clashes.empty()) {
                return std::make_pair(*flag_iter, *clashes.cbegin());
            }
            ++flag_iter;
        }

        // Now through parameters
        auto param_iter = param_names.cbegin();
        while (param_iter != param_names.cend()) {
            const auto& param = *param_iter;
            // Find if flag has any mutual exclusions
            auto [mutex_start, mutex_end] = this->pairs.equal_range(param);

            // Skip if not....
            if (std::distance(mutex_start, mutex_end) <= 0) {
                ++param_iter;
                continue;
            }
            // Names that could clash:
            std::set<ParamNameStr> excluded_names{};
            std::transform(mutex_start , mutex_end, std::inserter(excluded_names, excluded_names.begin()),
                           [](const auto& pair) { return pair.second; });

            // Check for clashes with flags
            std::set<ParamNameStr> clashes;
            std::set_intersection(excluded_names.cbegin(), excluded_names.cend(),
                                  flags.begin(), flags.cend(),
                                  std::inserter(clashes, clashes.begin()));

            // Check for clashes with parameter names
            std::set_intersection(excluded_names.cbegin(), excluded_names.cend(),
                                  param_names.cbegin(), param_names.cend(),
                                  std::inserter(clashes, clashes.end()));

            // If a clash, return matching flags
            if (!clashes.empty()) {
                return std::make_pair(*param_iter, *clashes.cbegin());
            }
            ++param_iter;
        }

        return {};
    }

    void MutuallyExclusiveParams::add_mutex(std::initializer_list<ParamNameStr> &&list) {
        // Do nothing, if not at least two in list
        if (list.size() < 2) {
            return;
        }

        // Triangle-iteration, registering every pair
        auto firstIter = list.begin();
        while (firstIter != list.end()) {
            auto secondIter = firstIter;
            ++secondIter;
            while (secondIter != list.end()) {
                this->add_mutex(*firstIter, *secondIter);
                ++secondIter;
            }
            ++firstIter;
        }
    }

    matlab::data::Array& SortedInputs::find_or_throw(const ParamNameStr& paramName) {
        auto param_iter = this->params.find(paramName);
        if (param_iter == this->params.end()) {
            std::stringstream ss;
            ss << "Parameter '" << UTF16toUTF8Convertor::convert_as_ascii(paramName)
               << "' should be specified.";
            throw errors::BadInput(errors::missing_param, ss.str());
        }
        return param_iter->second;
    }

    std::string SortedInputs::to_string() const {
        std::stringstream ss;
        if (!this->flags.empty()) {
            ss << "Flags set: ";
            bool one_flag = false;
            for (const auto& flag : this->flags) {
                if (one_flag) {
                    ss << ", ";
                }
                one_flag = true;
                ss << UTF16toUTF8Convertor::convert_as_ascii(flag);
            }
            ss << "\n";

        } else {
            ss << "No flags set.\n";
        }

        for (const auto& [param_name, val] : this->params) {
            ss << UTF16toUTF8Convertor::convert_as_ascii(param_name) << ": ";
            ss << summary_string(val);
            ss << "\n";
        }

        size_t index = 0;
        for (const auto& input : this->inputs) {
            ss << "Input " << (index+1) << ": "; // use matlab indexing
            ss << summary_string(input);
            ss << "\n";

            ++index;
        }

        return ss.str();
    }



}