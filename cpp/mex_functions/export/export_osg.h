/**
 * export_osg.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "exporter.h"
#include "utilities/io_parameters.h"

#include "MatlabDataArray.hpp"

namespace Moment {
    class OperatorSequenceGenerator;
    class SymbolTable;
}

namespace Moment::mex {
    class OSGExporter : public Exporter {
    private:
        mutable matlab::data::ArrayFactory factory;
        const SymbolTable& symbols;

    public:
        explicit OSGExporter(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols)
            : Exporter{engine}, symbols{symbols} { }

        matlab::data::CellArray operators(const OperatorSequenceGenerator& osg, bool offset = true) const;

        void sequences(IOArgumentRange& output, const OperatorSequenceGenerator& osg) const;

        void sequences_with_symbol_info(IOArgumentRange& output, const OperatorSequenceGenerator& osg) const;

    };
}