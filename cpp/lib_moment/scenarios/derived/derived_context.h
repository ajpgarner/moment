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

        DerivedContext(const Context& source_context);

    };

}
