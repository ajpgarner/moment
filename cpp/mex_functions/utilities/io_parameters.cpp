/**
 * io_parameters.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "operators/moment_matrix.h"
#include "operators/context.h"

#include "io_parameters.h"

#include "mex.hpp"

#include <algorithm>
#include <memory>

namespace NPATK::mex {

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

    matlab::data::Array& SortedInputs::find_or_throw(const ParamNameStr& paramName) {
        auto param_iter = this->params.find(paramName);
        if (param_iter == this->params.end()) {
            std::stringstream ss;
            ss << "Parameter '" << matlab::engine::convertUTF16StringToUTF8String(paramName)
               << "' should be specified.";
            throw errors::BadInput(errors::missing_param, ss.str());
        }
        return param_iter->second;
    }

    unsigned long SortedInputs::read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
                                                                  const std::string &paramName,
                                                                  const matlab::data::Array &array,
                                                                  unsigned long min_value) {
        if (!castable_to_scalar_int(array)) {
            std::stringstream ss;
            ss << paramName << " should be a scalar positive integer.";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }

        try {
            auto val = read_as_ulong(matlabEngine, array);
            if (val < min_value) {
                std::stringstream ss;
                ss << paramName << " must have a value of at least "
                   << min_value << ".";
                throw errors::BadInput{errors::bad_param, ss.str()};
            }
            return val;

        } catch (const errors::unreadable_scalar& use) {
            std::stringstream ss;
            ss << paramName << " could not be read: " << use.what();
            throw errors::BadInput{use.errCode, ss.str()};
        }
    }
}