/**
 * read_polynomial.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "read_polynomial.h"

#include "errors.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/utf_conversion.h"


namespace Moment::mex {
    [[nodiscard]] raw_sc_data
    read_raw_monomial_cell(matlab::engine::MATLABEngine& matlabEngine,
                      const std::string& fieldName, const matlab::data::CellArray& cell_input) {
        // Try to parse...
        size_t num_elems = cell_input.getNumberOfElements();
        if ((num_elems < 1) || (num_elems > 3)) {
            std::stringstream errSS;
            errSS << fieldName << " should have 1, 2 or 3 elements.";
            throw BadParameter{errSS.str()};
        }

        raw_sc_data output;
        output.symbol_id = read_as_scalar<uint64_t>(matlabEngine, cell_input[0]);
        if (num_elems > 1) {
            output.factor = read_as_complex_scalar<double>(matlabEngine, cell_input[1]);
            if (num_elems > 2) {
                output.conjugated = read_as_boolean(matlabEngine, cell_input[2]);
            } else {
                output.conjugated = false;
            }
        } else {
            output.factor = 1.0;
            output.conjugated = false;
        }
        return output;
    }

    raw_sc_data
    read_raw_monomial_string(matlab::engine::MATLABEngine& matlabEngine,
                      const std::string& fieldName, const matlab::data::MATLABString& input) {

        // If empty string, parse as zero
        if (!input.has_value()) {
            return {0, 0, false};
        }

        // Otherwise, attempt to read string
        const std::string input_string{UTF16toUTF8Convertor::convert_as_ascii(*input)};
        try {
            Monomial as_monomial{input_string};
            return raw_sc_data{static_cast<uint64_t>(as_monomial.id), as_monomial.factor, as_monomial.conjugated};
        } catch (const Monomial::SymbolParseException& spe) {
            // If parsing fails, report why
            std::stringstream errSS;
            errSS << fieldName << " could not be parsed: " << spe.what();
            throw BadParameter{errSS.str()};
        }
    }

    raw_sc_data read_raw_monomial(matlab::engine::MATLABEngine& matlabEngine,
                                  const std::string& fieldName, const matlab::data::Array& input) {

        switch (input.getType()) {
            case matlab::data::ArrayType::CELL:
                return read_raw_monomial_cell(matlabEngine, fieldName,
                                              static_cast<matlab::data::CellArray>(input));
            case matlab::data::ArrayType::MATLAB_STRING:
                // Empty string array -> parse as zero.
                if (input.isEmpty()) {
                    return raw_sc_data{0, 0, false};
                } else if (input.getNumberOfElements() == 1) {
                    auto sa = static_cast<matlab::data::StringArray>(input);
                    return read_raw_monomial_string(matlabEngine, fieldName, *sa.begin());
                } else {
                    std::stringstream errSS;
                    errSS << fieldName << " should be provided as a cell or string.";
                    throw BadParameter{errSS.str()};
                }
                break;
            default: {
                std::stringstream errSS;
                errSS << fieldName << " should be provided as a cell array or string.";
                throw BadParameter{errSS.str()};
            }
        }
    }

    namespace {
        std::vector<raw_sc_data>
        read_raw_polynomial_data_cell (matlab::engine::MATLABEngine& matlabEngine,
                                       const std::string& fieldName,
                                       const matlab::data::CellArray& cell_input) {
            auto raw_elem_num = cell_input.getNumberOfElements();

            // Prepare output
            std::vector<raw_sc_data> output;
            output.reserve(raw_elem_num);

            // Read each individual expression
            size_t index = 0;
            for (const auto& elem : cell_input) {
                std::stringstream elemSS;
                elemSS << fieldName << " element #" << (index+1);
                if (elem.getType() != matlab::data::ArrayType::CELL) {
                    elemSS << " was not a cell array!";
                    throw BadParameter{elemSS.str()};
                }

                output.emplace_back(read_raw_monomial_cell(matlabEngine, elemSS.str(), elem));
                ++index;
            }
            return output;
        }

        std::vector<raw_sc_data>
        read_raw_polynomial_data_string(matlab::engine::MATLABEngine& matlabEngine,
                                        const std::string& fieldName,
                                        const matlab::data::StringArray& string_input) {
            auto raw_elem_num = string_input.getNumberOfElements();

            // Prepare output
            std::vector<raw_sc_data> output;
            output.reserve(raw_elem_num);

            // Read each individual expression...
            size_t index = 0;
            for (const auto& elem : string_input) {
                std::stringstream elemSS;
                elemSS << fieldName << " element #" << (index+1);
                output.emplace_back(read_raw_monomial_string(matlabEngine, elemSS.str(), elem));
                ++index;
            }
            return output;
        }
    }


    std::vector<raw_sc_data>
    read_raw_polynomial_data(matlab::engine::MATLABEngine &matlabEngine,
                             const std::string& fieldName,
                             const matlab::data::Array &input) {

        switch (input.getType()) {
            case matlab::data::ArrayType::CELL:
                return read_raw_polynomial_data_cell(matlabEngine, fieldName,
                                                     static_cast<matlab::data::CellArray>(input));
            case matlab::data::ArrayType::MATLAB_STRING:
                return read_raw_polynomial_data_string(matlabEngine, fieldName,
                                                      static_cast<matlab::data::StringArray>(input));
            default: {
                std::stringstream errSS;
                errSS << fieldName << " should be provided as a cell or string array.";
                throw BadParameter{errSS.str()};
            }
        }

    }

    void check_raw_polynomial_data(matlab::engine::MATLABEngine &matlabEngine, const SymbolTable &symbols,
                                   std::span<const raw_sc_data> data) {
        size_t idx = 0;
        for (const auto& datum: data) {
            if (datum.symbol_id >= symbols.size()) {
                std::stringstream elemSS;
                elemSS << "Polynomial element #" << (idx + 1) << " contains symbol '" << datum.symbol_id
                       << "', which is out of range.";
                throw BadParameter{elemSS.str()};
            }
        }
        ++idx;
    }

    Polynomial raw_data_to_polynomial(matlab::engine::MATLABEngine &matlabEngine,
                                      const PolynomialFactory &factory, std::span<const raw_sc_data> data) {
        check_raw_polynomial_data(matlabEngine, factory.symbols, data);

        Polynomial::storage_t output_data;
        output_data.reserve(data.size());
        for (const auto& datum: data) {
            output_data.emplace_back(datum.symbol_id, datum.factor, datum.conjugated);
        }
        return factory(std::move(output_data));
    }

    Polynomial raw_data_to_polynomial_assume_sorted(matlab::engine::MATLABEngine &matlabEngine,
                                                    const PolynomialFactory &factory, std::span<const raw_sc_data> data) {
        // No check for range!

        Polynomial::storage_t output_data;
        output_data.reserve(data.size());
        for (const auto& datum: data) {
            output_data.emplace_back(datum.symbol_id, datum.factor, datum.conjugated);
        }
        // TODO: Creation without sort
        return factory(std::move(output_data));
    }
}