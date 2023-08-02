/**
 * contextual_os.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include <iosfwd>

#pragma once

namespace Moment {
    namespace Locality {
        class LocalityOperatorFormatter;
    }
    class Context;
    class SymbolTable;

    /**
     * Group of information
     */
    struct StringFormatContext {
    public:
        enum class DisplayAs : char {
            Operators,
            SymbolIds
        };

    public:
        /** Context. */
        const Context& context;

        /** Symbol table (optional) */
        const SymbolTable * symbols = nullptr;

        /** Extra formatting information. */
        struct FormattingInfo {
            /** Locality formatter (optional). */
            const Locality::LocalityOperatorFormatter * locality_formatter = nullptr;

            /** Switch between outputting operator sequences or symbol ids, for symbolic objects. */
            DisplayAs display_symbolic_as = DisplayAs::Operators;

            /** Show braces around operator sequences ('<X>' vs 'X') */
            bool show_braces = false;

            /** Show hash  in front of symbol number ('#2' vs '2') */
            bool hash_before_symbol_id = false;

            /** True monomial should be formatted as if at front of a polynomial, false otherwise. */
            bool first_in_polynomial = true;

        } format_info;

        /** Bind stream to context. */
        explicit constexpr StringFormatContext(const Context& context) : context{context} { }

        /** Bind stream to context and symbol table. */
        constexpr StringFormatContext(const Context& context, const SymbolTable& symbols)
        : context{context}, symbols{&symbols} { }

        /** Bind new output stream, with settings previous stream */
        constexpr StringFormatContext(const StringFormatContext& reference) = default;
    };

    /**
     * Binds an output stream together with a reference to context for additional formatting information.
     */
    class ContextualOS : public StringFormatContext {
    public:
        /** Underlying operator stream */
        std::ostream& os;

        /** Bind stream to context. */
        constexpr ContextualOS(std::ostream& os, const Context& context)
            : StringFormatContext{context}, os{os} { }

        /** Bind stream to context and symbol table. */
        constexpr ContextualOS(std::ostream& os, const Context& context, const SymbolTable& symbols)
            : StringFormatContext{context, symbols}, os{os} { }

        /** Bind new output stream, with settings previous stream */
        constexpr ContextualOS(std::ostream& os, const StringFormatContext& reference)
            : StringFormatContext{reference}, os{os} { }

        /**
         * Explicit cast down to underlying operator stream, if no ContextualOS definition provided.
         */
        [[nodiscard]] operator std::ostream&() noexcept { // NOLINT(*-explicit-constructor)
            return this->os;
        }

        /**
         * Default match for <<, just output to underlying OS.
         */
        template<typename object_t>
        friend ContextualOS& operator<<(ContextualOS& contextual_os, const object_t& object) {
            contextual_os.os << object;
            return contextual_os;
        }

    };


}