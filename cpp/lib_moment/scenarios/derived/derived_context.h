/**
 * derived_context.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../context.h"
#include "../contextual_os.h"

#include <optional>

namespace Moment::Derived {

    class SymbolTableMap;

    class DerivedContext : public Context {
    public:
        const Context& base_context;

    private:
        std::optional<StringFormatContext> sfc;
        const SymbolTableMap * map_ptr = nullptr;

    public:

        DerivedContext(const Context& source_context);

        void format_sequence_from_symbol_id(ContextualOS& os,
                                            const symbol_name_t symbol_id,
                                            bool conjugated) const override;

        inline const SymbolTableMap& SymbolTableMap() const noexcept {
            assert(this->map_ptr);
            return *this->map_ptr;
        }

        friend class DerivedMatrixSystem;

    private:
        void set_symbol_table_map(const class SymbolTableMap * new_map_ptr) noexcept;
    };

}
