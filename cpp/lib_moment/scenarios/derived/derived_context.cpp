/**
 * derived_context.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "derived_context.h"

namespace Moment::Derived {

    DerivedContext::DerivedContext(const Context& source_context)
        : Context(0), base_context{source_context} {

    }


}