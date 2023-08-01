/**
 * derived_context.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../context.h"

namespace Moment::Derived {

    class DerivedContext : public Context {
    public:
        const Context& base_context;

    private:
        std::vector<std::string> derived_symbol_strs;


    public:

        DerivedContext(const Context& source_context);

        void format_symbol(std::ostream &os, const SymbolTable &table, const symbol_name_t symbol_id) const override;

        friend class DerivedMatrixSystem;
    };

}
