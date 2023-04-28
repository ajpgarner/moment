/**
 * export_symbol_combo.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"

#include "MatlabDataArray.hpp"

namespace Moment {
    class SymbolTable;
    class SymbolCombo;
}

namespace Moment::mex {
    class SymbolComboExporter : public Exporter {
    public:
        const SymbolTable& symbols;

        explicit SymbolComboExporter(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols) noexcept
                    : Exporter{engine}, symbols{symbols} { }

        std::pair<matlab::data::Array, matlab::data::Array> operator()(const SymbolCombo& combo) const;
    };
}