/**
 * export_collins_gisin.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "exporter.h"

#include "MatlabDataArray.hpp"

namespace Moment {
    class SymbolTable;
    class CollinsGisin;
    class Context;

    namespace Locality {
        class LocalityContext;
        class LocalityOperatorFormatter;
    }

    namespace mex {
        class CollinsGisinExporter : Exporter {
        public:
            const Context& context;
            const SymbolTable& symbols;

            CollinsGisinExporter(matlab::engine::MATLABEngine &engine,
                                 const Context &context, const SymbolTable& symbols);

            [[nodiscard]] std::pair<matlab::data::TypedArray<uint64_t>,
                                    matlab::data::TypedArray<int64_t>>
            symbol_and_basis(const Moment::CollinsGisin& cgi) const;

            [[nodiscard]] std::pair<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>>
            sequence_and_hash(const Moment::CollinsGisin& cgi) const;

            [[nodiscard]] std::tuple<matlab::data::CellArray, matlab::data::TypedArray<uint64_t>,
                                     matlab::data::TypedArray<uint64_t>,
                                     matlab::data::TypedArray<int64_t>>
            everything(const Moment::CollinsGisin& cgi) const;

            [[nodiscard]] matlab::data::StringArray
            strings(const Moment::CollinsGisin& cg,
                    const Moment::Locality::LocalityOperatorFormatter& formatter) const;

            [[nodiscard]] matlab::data::StringArray
            strings(const Moment::CollinsGisin& cg) const;
        };
    }
}