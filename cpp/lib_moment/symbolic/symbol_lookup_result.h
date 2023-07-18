/**
 * symbol_lookup_result.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "symbol.h"
#include "dictionary/operator_sequence.h"

#include <cassert>
#include <optional>

namespace Moment {

    /**
     * Result of SymbolTable.where(sequence).
     */
    struct SymbolLookupResult {
    public:
        /**
         * Pointer to symbol found in table, or nullptr if not found.
         */
        const Symbol * symbol = nullptr;

        /**
         * True if sequence supplied is conjugated w.r.t. to entry in symbol table.
         */
        bool is_conjugated = false;

        /**
         * True if sequence supplied was not found directly, but instead via a moment simplification alias.
         * Also true if sequence was not found, but could be determined to not correspond to a canonical moment.
         * Otherwise false.
         */
        bool is_aliased = false;

    public:
        constexpr SymbolLookupResult() noexcept = default;

        constexpr SymbolLookupResult(const Symbol * symbol, bool is_conjugated, bool is_aliased) noexcept
            : symbol{symbol}, is_conjugated{is_conjugated}, is_aliased{is_aliased} { }

        [[nodiscard]] constexpr bool found() const noexcept { return symbol != nullptr; }

        [[nodiscard]] constexpr const Symbol& operator*() const noexcept {
            assert(this->symbol != nullptr);
            return *this->symbol;
        }

        [[nodiscard]] constexpr const Symbol* operator->() const noexcept {
            assert(this->symbol != nullptr);
            return this->symbol;
        }

        /** Override so that "*this == nullptr" works. */
        [[nodiscard]] constexpr bool operator==(nullptr_t /**/) const noexcept {
            return this->symbol == nullptr;
        }

        /** Override so that "*this == nullptr" works. */
        [[nodiscard]] constexpr bool operator!=(nullptr_t /**/) const noexcept {
            return this->symbol != nullptr;
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept {
            return this->symbol != nullptr;
        }

        [[nodiscard]] constexpr bool operator!() const noexcept {
            return this->symbol == nullptr;
        }

    };
}