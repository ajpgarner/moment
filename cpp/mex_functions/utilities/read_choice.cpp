/**
 * read_choice.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_choice.h"

#include "mex.hpp"

#include "utilities/utf_conversion.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace Moment::mex {

    /** Returns whether input represents a string that matches one of the choices given*/
    size_t read_choice(const std::string& param_name, std::vector<std::string> choices, matlab::data::Array input) {

        // Parse input to utf8 string
        std::string input_choice;
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            auto as_ca = static_cast<matlab::data::CharArray>(input);
            input_choice = as_ca.toAscii();
        } else if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            if (input.getNumberOfElements() != 1) {
                std::stringstream errSS;
                errSS << param_name << " must be a single string.";
            }
            matlab::data::TypedArray<matlab::data::MATLABString> as_mls_array{std::move(input)};
            auto mls = *(as_mls_array.cbegin());
            if (!mls.has_value()) {
                std::stringstream errSS;
                errSS << param_name << " must be a single not-null string.";
            }
            UTF16toUTF8Convertor convertor;

            input_choice = convertor(*mls);
        } else {
            std::stringstream errSS;
            errSS << param_name << " must be a string.";
            throw errors::invalid_choice{errSS.str()};
        }

        // Make choice lower case
        std::transform(input_choice.begin(), input_choice.end(), input_choice.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        // Compare
        for (size_t index = 0; index < choices.size(); ++index) {
            if (input_choice == choices[index]) {
                return index;
            }
        }

        // Complain if no match
        std::stringstream errSS;
        errSS << param_name << " value '" << input_choice << "' not recognized. Must be one of: ";
        bool done_once = false;
        for (const auto& str : choices) {
            if (done_once) {
                errSS << ", ";
            } else {
                done_once = true;
            }
            errSS << "'" << str << "'";
        }
        errSS << ".";
        throw errors::invalid_choice{errSS.str()};
    }

}
