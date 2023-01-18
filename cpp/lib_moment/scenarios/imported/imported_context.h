/**
 * imported_context.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
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
    };
}