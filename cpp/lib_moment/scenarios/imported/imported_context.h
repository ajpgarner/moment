/**
 * imported_context.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "scenarios/context.h"

namespace Moment::Imported {
    class ImportedContext : public Context {
    private:
        const bool purely_real = false;

    public:
        explicit ImportedContext(bool real_only = false);

        /** Imported context can be non-hermitian, unless "purely real" is set */
        bool can_be_nonhermitian() const noexcept override;

        /** True if all operator sequences in context must be real */
        [[nodiscard]] bool real_only() const noexcept { return this->purely_real; };

        /** Imported context has no operators */
        [[nodiscard]] bool defines_operators() const noexcept final {
            return false;
        };
    };
}