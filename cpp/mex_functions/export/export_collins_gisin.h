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

    namespace Locality {
        class LocalityContext;
        class LocalityOperatorFormatter;
        class CollinsGisin;
    }

    namespace mex {
        class CollinsGisinExporter : Exporter {
        public:
            const Moment::Locality::LocalityContext &context;
            const SymbolTable &symbols;

            CollinsGisinExporter(matlab::engine::MATLABEngine &engine,
                                 const Moment::Locality::LocalityContext &context, const SymbolTable& symbols)
                    : Exporter{engine}, context{context}, symbols{symbols} { }

            [[nodiscard]] matlab::data::TypedArray<uint64_t> symbol_ids(const Moment::Locality::CollinsGisin& cg) const;

            [[nodiscard]] matlab::data::TypedArray<int64_t> basis_elems(const Moment::Locality::CollinsGisin& cg) const;

            [[nodiscard]] matlab::data::CellArray sequences(const Moment::Locality::CollinsGisin& cg) const;

            [[nodiscard]] matlab::data::TypedArray<uint64_t> hashes(const Moment::Locality::CollinsGisin& cg) const;

            [[nodiscard]] matlab::data::StringArray
            strings(const Moment::Locality::CollinsGisin& cg,
                    const Moment::Locality::LocalityOperatorFormatter& formatter) const;
        };
    }
}