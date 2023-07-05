/**
 * export_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_collins_gisin.h"

#include "export_operator_sequence.h"

#include "probability/collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "utilities/utf_conversion.h"

namespace Moment::mex {

    CollinsGisinExporter::CollinsGisinExporter(matlab::engine::MATLABEngine &engine,
                                               const Context &context, const SymbolTable &symbols)
            : Exporter{engine}, context{context}, symbols{symbols} { }


    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<int64_t>>
    CollinsGisinExporter::symbol_and_basis(const Moment::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        auto output = std::make_pair(factory.createArray<uint64_t>(dimensions),
                                     factory.createArray<int64_t>(dimensions));

        // Check before (explicit mode):
        if (!cgi.HasAllSymbols()) {
            throw Moment::errors::BadCGError::make_missing_err(cgi);
        }

        auto writeSymbolIter = output.first.begin();
        auto writeBasisIter = output.second.begin();
        auto readIter = cgi.begin();
        const auto readEnd = cgi.end();
        while ((writeSymbolIter != output.first.end()) && (writeBasisIter != output.second.end())
                && (readIter != readEnd)) {

            // Fail within (virtual mode):
            if (-1 == readIter->symbol_id) {
                throw Moment::errors::BadCGError::make_missing_index_err(readIter.index(), readIter->sequence, true);
            }

            *writeSymbolIter = readIter->symbol_id;
            *writeBasisIter = readIter->real_index;
            ++writeSymbolIter;
            ++writeBasisIter;
            ++readIter;
        }

        return output;
    }

    std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>>
    CollinsGisinExporter::sequence_and_hash(const Moment::CollinsGisin &cgi) const {
        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};

        auto output = std::make_pair(factory.createCellArray(dimensions),
                                     factory.createArray<uint64_t>(dimensions));

        auto writeSequenceIter = output.first.begin();
        auto writeHashIter = output.second.begin();


        auto readIter = cgi.begin();
        const auto readEnd = cgi.end();

        while ((writeSequenceIter != output.first.end()) && (writeHashIter != output.second.end())
                && (readIter != readEnd)) {
            *writeSequenceIter = export_operator_sequence(factory, readIter->sequence, true);
            *writeHashIter = readIter->sequence.hash();

            ++writeSequenceIter;
            ++writeHashIter;
            ++readIter;
        }
        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Moment::CollinsGisin& cgi,
                                  const Locality::LocalityOperatorFormatter& formatter) const {
        auto lcPtr = dynamic_cast<const Locality::LocalityContext*>(&this->context);
        if (lcPtr == nullptr) {
            return (this->strings(cgi));
        }
        const auto& localityContext = *lcPtr;

        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));

        auto writeIter = output.begin();
        auto readIter = cgi.begin();
        const auto readEnd = cgi.end();;
        while (writeIter != output.end() && readIter != readEnd) {
            std::string inStr{localityContext.format_sequence(formatter, readIter->sequence)};
            *writeIter = UTF8toUTF16Convertor::convert(inStr);
            ++writeIter;
            ++readIter;
        }
        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Moment::CollinsGisin& cgi) const {

        matlab::data::ArrayFactory factory;

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));

        auto writeIter = output.begin();
        auto readIter = cgi.begin();
        const auto readEnd = cgi.end();
        while (writeIter != output.end() && readIter != readEnd) {
            std::string inStr{this->context.format_sequence(readIter->sequence)};
            *writeIter = UTF8toUTF16Convertor::convert(inStr);
            ++writeIter;
            ++readIter;
        }
        return output;
    }

}