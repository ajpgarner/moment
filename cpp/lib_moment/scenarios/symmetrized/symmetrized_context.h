/**
 * symmetrized_context.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../context.h"

namespace Moment::Symmetrized {

    class SymmetrizedContext : public Context {
    public:
        const Context& base_context;

        SymmetrizedContext(const Context& source_context);

    };

}
