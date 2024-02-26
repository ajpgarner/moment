/**
 * read_operator_names.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_operator_names.h"

#include "errors.h"
#include "utilities/reporting.h"

#include "utilities/utf_conversion.h"

namespace Moment::mex {

    std::unique_ptr<Algebraic::NameTable>
    read_name_table(matlab::engine::MATLABEngine &matlabEngine,
                    const Algebraic::AlgebraicPrecontext& apc,
                    const std::string& paramName,
                    matlab::data::Array &input) {
        std::unique_ptr<Algebraic::NameTable> output;

        // Is operator argument a single string?
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            std::vector<std::string> raw_names;

            auto name_char_array = static_cast<matlab::data::CharArray>(input);
            auto name_str = name_char_array.toAscii();
            raw_names.reserve(name_str.size());
            for (auto x : name_str) {
                raw_names.emplace_back(1, x);
            }
            try {
                output = std::make_unique<Algebraic::NameTable>(apc, std::move(raw_names));
            } catch (const std::invalid_argument& iae) {
                std::stringstream errSS;
                errSS << paramName << " could not be parsed: " << iae.what();
                throw BadParameter{errSS.str()};
            }
            return output;
        }

        // Is operator argument an array of strings?
        if (input.getType() == matlab::data::ArrayType::MATLAB_STRING)
        {
            UTF16toUTF8Convertor convertor{};

            std::vector<std::string> raw_names;
            auto mls_array = static_cast<matlab::data::TypedArray<matlab::data::MATLABString>>(input);
            raw_names.reserve(mls_array.getNumberOfElements());
            for (auto elem : mls_array) {
                if (elem.has_value()) {
                    auto utf8str = convertor(elem);
                    raw_names.emplace_back(utf8str);
                }
            }

            try {
                output = std::make_unique<Algebraic::NameTable>(apc, std::move(raw_names));
            } catch (const std::invalid_argument& iae) {
                std::stringstream errSS;
                errSS << paramName << " could not be parsed: " << iae.what();
                throw BadParameter{errSS.str()};
            }
            return output;
        }

        std::stringstream errSS;
        errSS << paramName << " could not be parsed: name table must be char array or string array.";
        throw BadParameter{errSS.str()};
    }

    size_t get_name_table_length(matlab::engine::MATLABEngine &matlabEngine, const std::string &paramName,
                                 matlab::data::Array &input) {
        // Is operator argument a single string?
        if (input.getType() == matlab::data::ArrayType::CHAR) {
            auto name_char_array = static_cast<matlab::data::CharArray>(input);
            auto name_str = name_char_array.toAscii();
            return name_str.size();
        }

        // Is operator argument an array of ML strings?
        if (input.getType() == matlab::data::ArrayType::MATLAB_STRING) {
            return input.getNumberOfElements();
        }

        // Otherwise, error
        std::stringstream errSS;
        errSS << paramName << " could not be parsed: name table must be char array or string array.";
        throw BadParameter{errSS.str()};
    }
}