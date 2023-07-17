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
        class FactorTable;
    }

    namespace Locality {
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
        const bool include_factors;
        const bool locality_format;

        const Inflation::FactorTable * factorTablePtr = nullptr;
        const Locality::LocalityContext * localityContextPtr = nullptr;

    public:

        SymbolTableExporter(matlab::engine::MATLABEngine& engine, const EnvironmentalVariables& env,
                            const MatrixSystem& system);

        [[nodiscard]] std::vector<std::string> column_names(bool include_conj) const;

        [[nodiscard]] matlab::data::StructArray export_empty_row(bool include_conj) const;

        [[nodiscard]] matlab::data::StructArray export_row(const Symbol& symbol,
                                                           std::optional<bool> conjugated = std::nullopt) const;

        [[nodiscard]] matlab::data::StructArray export_table(size_t from_symbol = 0) const;

        [[nodiscard]] matlab::data::StructArray export_row_array(std::span<const size_t> shape,
                                                                 std::span<const symbol_name_t> symbol_ids,
                                                                 std::span<const uint8_t> conj_status) const;

        [[nodiscard]] matlab::data::StructArray export_row_array(std::span<const symbol_name_t> symbol_ids,
                                                                 std::span<const uint8_t> conj_status) const {
            std::vector<size_t> dimensions{1, symbol_ids.size()};
            return export_row_array(dimensions, symbol_ids, conj_status);
        }

    private:
        void do_row_write(matlab::data::StructArray::iterator& write_iter,
                          const Symbol& symbol_info, std::optional<bool> conjugated) const;

        void do_missing_row_write(matlab::data::StructArray::iterator& write_iter, bool include_conj) const;

    };


}
