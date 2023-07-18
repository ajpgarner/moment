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

#include "utilities/iter_tuple.h"
#include "utilities/utf_conversion.h"

namespace Moment::mex {


    namespace {
        struct SymbolInfoWriter {
            std::tuple<uint64_t, int64_t> operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                // Fail within (virtual mode):
                if (-1 == element.symbol_id) {
                    throw Moment::errors::BadCGError::make_missing_index_err(index, element.sequence, true);
                }

                return {element.symbol_id, element.real_index};
            }
        };

        struct SymbolInfoAliasedWriter {
            std::tuple<uint64_t, int64_t, bool> operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                // Fail within (virtual mode):
                if (-1 == element.symbol_id) {
                    throw Moment::errors::BadCGError::make_missing_index_err(index, element.sequence, true);
                }

                return {element.symbol_id, element.real_index, element.is_alias};
            }
        };

        struct SequenceWriter {
            matlab::data::ArrayFactory& factory;
            explicit SequenceWriter(matlab::data::ArrayFactory& factory) : factory{factory} { }

            std::tuple<matlab::data::TypedArray<uint64_t>, uint64_t>
            operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                return {export_operator_sequence(factory, element.sequence, true), element.sequence.hash()};
            }
        };

        struct EverythingWriter {
            matlab::data::ArrayFactory& factory;
            explicit EverythingWriter(matlab::data::ArrayFactory& factory) : factory{factory} { }

            std::tuple<matlab::data::TypedArray<uint64_t>, uint64_t, uint64_t, int64_t>
            operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                // Fail within (virtual mode):
                if (-1 == element.symbol_id) {
                    throw Moment::errors::BadCGError::make_missing_index_err(index, element.sequence, true);
                }

                return {export_operator_sequence(factory, element.sequence, true), element.sequence.hash(),
                        element.symbol_id, element.real_index};
            }
        };

        struct EverythingAliasedWriter {
            matlab::data::ArrayFactory& factory;
            explicit EverythingAliasedWriter(matlab::data::ArrayFactory& factory) : factory{factory} { }

            std::tuple<matlab::data::TypedArray<uint64_t>, uint64_t, uint64_t, int64_t, bool>
            operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                // Fail within (virtual mode):
                if (-1 == element.symbol_id) {
                    throw Moment::errors::BadCGError::make_missing_index_err(index, element.sequence, true);
                }

                return {export_operator_sequence(factory, element.sequence, true), element.sequence.hash(),
                        element.symbol_id, element.real_index, element.is_alias};
            }
        };

        struct LFStringWriter {
            const Locality::LocalityContext& localityContext;
            const Locality::LocalityOperatorFormatter& formatter;
        public:
            LFStringWriter(const Locality::LocalityContext& localityContext,
                           const Locality::LocalityOperatorFormatter& formatter)
                : localityContext{localityContext}, formatter{formatter} {

            }

            std::basic_string<char16_t>
            operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                std::string inStr{localityContext.format_sequence(formatter, element.sequence)};
                return UTF8toUTF16Convertor::convert(inStr);
            }
        };

        struct StringWriter {
            const Context& context;
        public:
            explicit StringWriter(const Context& context) : context{context} {}


            std::basic_string<char16_t>
            operator()(CollinsGisinIndexView index, const CollinsGisinEntry& element) const {
                std::string inStr{this->context.format_sequence(element.sequence)};
                return UTF8toUTF16Convertor::convert(inStr);
            }
        };


        template<typename read_iter_t, typename write_iter_t, typename functor_t>
        void indexed_transform(read_iter_t read_iter, const read_iter_t read_iter_end,
                               write_iter_t write_iter, const functor_t& functor) {
            while (read_iter != read_iter_end) {
                *write_iter = functor(read_iter.index(), *read_iter);
                ++read_iter;
                ++write_iter;
            }
        }
    }


    CollinsGisinExporter::CollinsGisinExporter(matlab::engine::MATLABEngine &engine,
                                               const Context &context, const SymbolTable &symbols)
            : ExporterWithFactory{engine}, context{context}, symbols{symbols} { }


    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<int64_t>>
    CollinsGisinExporter::symbol_and_basis(const Moment::CollinsGisin &cgi) const {
        // Check before (explicit mode):
        if (!cgi.HasAllSymbols()) {
            throw Moment::errors::BadCGError::make_missing_err(cgi);
        }

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        auto output = std::make_pair(factory.createArray<uint64_t>(dimensions),
                                     factory.createArray<int64_t>(dimensions));
        IterTuple write_iter{output.first.begin(), output.second.begin()};
        indexed_transform(cgi.begin(), cgi.end(), write_iter, SymbolInfoWriter{});

        return output;
    }

    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<int64_t>>
    CollinsGisinExporter::symbol_and_basis(const Moment::CollinsGisinRange &cgr) const {

        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        auto output = std::make_pair(factory.createArray<uint64_t>(dimensions),
                                     factory.createArray<int64_t>(dimensions));
        IterTuple write_iter{output.first.begin(), output.second.begin()};
        indexed_transform(cgr.begin(), cgr.end(), write_iter, SymbolInfoWriter{});

        return output;
    }

    std::tuple<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<int64_t>, matlab::data::TypedArray<bool>>
    CollinsGisinExporter::symbol_basis_and_alias(const Moment::CollinsGisin &cgi) const {
        // Check before (explicit mode):
        if (!cgi.HasAllSymbols()) {
            throw Moment::errors::BadCGError::make_missing_err(cgi);
        }

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        auto output = std::make_tuple(factory.createArray<uint64_t>(dimensions),
                                      factory.createArray<int64_t>(dimensions),
                                      factory.createArray<bool>(dimensions));
        IterTuple write_iter{std::get<0>(output).begin(), std::get<1>(output).begin(), std::get<2>(output).begin()};
        indexed_transform(cgi.begin(), cgi.end(), write_iter, SymbolInfoAliasedWriter{});

        return output;
    }

    std::tuple<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<int64_t>, matlab::data::TypedArray<bool>>
    CollinsGisinExporter::symbol_basis_and_alias(const Moment::CollinsGisinRange &cgr) const {

        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        auto output = std::make_tuple(factory.createArray<uint64_t>(dimensions),
                                      factory.createArray<int64_t>(dimensions),
                                      factory.createArray<bool>(dimensions));
        IterTuple write_iter{std::get<0>(output).begin(), std::get<1>(output).begin(), std::get<2>(output).begin()};
        indexed_transform(cgr.begin(), cgr.end(), write_iter, SymbolInfoAliasedWriter{});

        return output;
    }

    std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>>
    CollinsGisinExporter::sequence_and_hash(const Moment::CollinsGisin& cgi) const {
        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};

        std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>>
            output{factory.createCellArray(dimensions), factory.createArray<uint64_t>(dimensions)};

        auto cell_iter = output.first.begin();

        IterTuple write_iter{output.first.begin(), output.second.begin()};
        indexed_transform(cgi.begin(), cgi.end(), write_iter, SequenceWriter{factory});

        return output;
    }

    std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>>
    CollinsGisinExporter::sequence_and_hash(const Moment::CollinsGisinRange& cgr) const {
        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>> output =
                {factory.createCellArray(dimensions), factory.createArray<uint64_t>(dimensions)};

        IterTuple write_iter{output.first.begin(), output.second.begin()};
        indexed_transform(cgr.begin(), cgr.end(), write_iter, SequenceWriter{factory});

        return output;
    }

    std::tuple<matlab::data::CellArray,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<int64_t>>
   CollinsGisinExporter::everything(const CollinsGisin &cgi) const {
        matlab::data::ArrayDimensions  dimensions{cgi.Dimensions};

        // Check before (explicit mode):
        if (!cgi.HasAllSymbols()) {
            throw Moment::errors::BadCGError::make_missing_err(cgi);
        }

        auto output = std::make_tuple(
                factory.createCellArray(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<int64_t>(dimensions)
            );

        IterTuple write_iter{std::get<0>(output).begin(),
                             std::get<1>(output).begin(),
                             std::get<2>(output).begin(),
                             std::get<3>(output).begin()};
        indexed_transform(cgi.begin(), cgi.end(), write_iter, EverythingWriter{factory});
        return output;
    }
    
    std::tuple<matlab::data::CellArray,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<int64_t>>
   CollinsGisinExporter::everything(const CollinsGisinRange &cgr) const {
        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        auto output = std::make_tuple(
                factory.createCellArray(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<int64_t>(dimensions)
            );

        IterTuple write_iter{std::get<0>(output).begin(),
                             std::get<1>(output).begin(),
                             std::get<2>(output).begin(),
                             std::get<3>(output).begin()};
        indexed_transform(cgr.begin(), cgr.end(), write_iter, EverythingWriter{factory});
        return output;
    }

    std::tuple<matlab::data::CellArray,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<int64_t>,
               matlab::data::TypedArray<bool>>
   CollinsGisinExporter::everything_with_aliases(const CollinsGisin &cgi) const {
        matlab::data::ArrayDimensions  dimensions{cgi.Dimensions};

        // Check before (explicit mode):
        if (!cgi.HasAllSymbols()) {
            throw Moment::errors::BadCGError::make_missing_err(cgi);
        }

        auto output = std::make_tuple(
                factory.createCellArray(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<int64_t>(dimensions),
                factory.createArray<bool>(dimensions)
            );

        IterTuple write_iter{std::get<0>(output).begin(),
                             std::get<1>(output).begin(),
                             std::get<2>(output).begin(),
                             std::get<3>(output).begin(),
                             std::get<4>(output).begin()};
        indexed_transform(cgi.begin(), cgi.end(), write_iter, EverythingAliasedWriter{factory});
        return output;
    }

    std::tuple<matlab::data::CellArray,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<uint64_t>,
               matlab::data::TypedArray<int64_t>,
               matlab::data::TypedArray<bool>>
   CollinsGisinExporter::everything_with_aliases(const CollinsGisinRange &cgr) const {
        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        auto output = std::make_tuple(
                factory.createCellArray(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<uint64_t>(dimensions),
                factory.createArray<int64_t>(dimensions),
                factory.createArray<bool>(dimensions)
            );

        IterTuple write_iter{std::get<0>(output).begin(),
                             std::get<1>(output).begin(),
                             std::get<2>(output).begin(),
                             std::get<3>(output).begin(),
                             std::get<4>(output).begin()};
        indexed_transform(cgr.begin(), cgr.end(), write_iter, EverythingAliasedWriter{factory});
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

        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));
        indexed_transform(cgi.begin(), cgi.end(), output.begin(), LFStringWriter{localityContext, formatter});

        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Moment::CollinsGisinRange& cgr,
                                  const Locality::LocalityOperatorFormatter& formatter) const {
        auto lcPtr = dynamic_cast<const Locality::LocalityContext*>(&this->context);
        if (lcPtr == nullptr) {
            return (this->strings(cgr));
        }
        const auto& localityContext = *lcPtr;

        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));
        indexed_transform(cgr.begin(), cgr.end(), output.begin(), LFStringWriter{localityContext, formatter});

        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Moment::CollinsGisin& cgi) const {
        matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));
        indexed_transform(cgi.begin(), cgi.end(), output.begin(), StringWriter{this->context});

        return output;
    }

    matlab::data::StringArray
    CollinsGisinExporter::strings(const Moment::CollinsGisinRange& cgr) const {
        matlab::data::ArrayDimensions dimensions(cgr.Dimensions().begin(), cgr.Dimensions().end());
        matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));
        indexed_transform(cgr.begin(), cgr.end(), output.begin(), StringWriter{this->context});

        return output;
    }

}