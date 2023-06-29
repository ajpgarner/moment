/**
 * export_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_collins_gisin.h"

#include "export_operator_sequence.h"

#include "scenarios/locality/collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "utilities/utf_conversion.h"

namespace Moment::mex {

    matlab::data::TypedArray<uint64_t>  CollinsGisinExporter::symbol_ids(const Locality::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        // Could throw, if CGI is incomplete.
        const auto& readData = cgi.Symbols();

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<uint64_t> output = factory.createArray<uint64_t>(std::move(dimensions));

        auto writeIter = output.begin();
        auto readIter = readData.begin();
        while (writeIter != output.end() && readIter != readData.end()) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }

        return output;

    }

    matlab::data::TypedArray<int64_t>
    CollinsGisinExporter::basis_elems(const Moment::Locality::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        // Could throw, if CGI is incomplete.
        const auto& readData = cgi.RealIndices();

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<int64_t> output = factory.createArray<int64_t>(std::move(dimensions));

        auto writeIter = output.begin();
        auto readIter = readData.begin();
        while (writeIter != output.end() && readIter != readData.end()) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }

        return output;

    }

    matlab::data::CellArray CollinsGisinExporter::sequences(const Locality::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::CellArray output = factory.createCellArray(std::move(dimensions));

        const auto& readData = cgi.Sequences();
        auto writeIter = output.begin();
        auto readIter = readData.begin();
        while (writeIter != output.end() && readIter != readData.end()) {
            *writeIter = export_operator_sequence(factory, *readIter, true);
            ++writeIter;
            ++readIter;
        }
        return output;
    }

    matlab::data::TypedArray<uint64_t> CollinsGisinExporter::hashes(const Locality::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<uint64_t> output = factory.createArray<uint64_t>(std::move(dimensions));

        const auto& readData = cgi.Sequences();
        auto writeIter = output.begin();
        auto readIter = readData.begin();
        while (writeIter != output.end() && readIter != readData.end()) {
            *writeIter = readIter->hash();
            ++writeIter;
            ++readIter;
        }
        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Locality::CollinsGisin& cgi,
                                  const Locality::LocalityOperatorFormatter& formatter) const {
        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));

        const auto& readData = cgi.Sequences();
        auto writeIter = output.begin();
        auto readIter = readData.begin();
        while (writeIter != output.end() && readIter != readData.end()) {

            std::string inStr{this->context.format_sequence(formatter, *readIter)};
            *writeIter = UTF8toUTF16Convertor::convert(inStr);
            ++writeIter;
            ++readIter;
        }

        return output;

    }
}