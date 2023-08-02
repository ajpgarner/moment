/**
 * export_symbol_table.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "exporter.h"
#include "integer_types.h"
#include "symbolic/symbol_lookup_result.h"

#include "scenarios/contextual_os.h"

#include <optional>
#include <span>
#include <vector>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class Context;
    class MatrixSystem;
    class Symbol;
    class SymbolTable;

    namespace Inflation {
        class InflationMatrixSystem;
        class FactorTable;
    }

    namespace Locality {
        class LocalityMatrixSystem;
        class LocalityContext;
    }
}

namespace Moment::mex {
    class EnvironmentalVariables;

    class SymbolTableExporter : public ExporterWithFactory {
    public:
        const EnvironmentalVariables& env;
        const MatrixSystem& system;
        const SymbolTable& symbols;
        const Context& context;

    private:
        StringFormatContext sfContext;

    public:
        const bool include_factors;
        const bool locality_format;
        const bool can_have_aliases;

    private:
        const Inflation::FactorTable * factorTablePtr = nullptr;
        const Locality::LocalityContext * localityContextPtr = nullptr;



    public:

        SymbolTableExporter(matlab::engine::MATLABEngine& engine, const EnvironmentalVariables& env,
                            const MatrixSystem& system);

        SymbolTableExporter(matlab::engine::MATLABEngine& engine, const EnvironmentalVariables& env,
                            const Locality::LocalityMatrixSystem& system);

        SymbolTableExporter(matlab::engine::MATLABEngine& engine, const EnvironmentalVariables& env,
                            const Inflation::InflationMatrixSystem& system);

        [[nodiscard]] std::vector<std::string> column_names(bool look_up_mode) const;

        [[nodiscard]] matlab::data::StructArray export_empty_row(bool look_up_mode) const;

        [[nodiscard]] matlab::data::StructArray export_row(const Symbol& symbol,
                                                           std::optional<bool> conjugated = std::nullopt,
                                                           std::optional<bool> is_alias = std::nullopt) const;

        [[nodiscard]] matlab::data::StructArray export_table(size_t from_symbol = 0) const;

        [[nodiscard]] matlab::data::StructArray
        export_row_array(std::span<const size_t> shape, std::span<const SymbolLookupResult> symbol_info) const;

        [[nodiscard]] matlab::data::StructArray
        inline export_row_array(const std::span<const SymbolLookupResult> symbol_info) const {
            std::vector<size_t> dimensions{1, symbol_info.size()};
            return export_row_array(dimensions, symbol_info);
        }

    private:
        void do_row_write(matlab::data::StructArray::iterator& write_iter,
                          const Symbol& symbol_info,
                          std::optional<bool> conjugated,
                          std::optional<bool> aliased) const;

        void do_missing_row_write(matlab::data::StructArray::iterator& write_iter, bool look_up_mode) const;

    };


}
