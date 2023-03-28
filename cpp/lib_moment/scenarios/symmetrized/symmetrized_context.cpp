/**
 * symmetrized_context.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symmetrized_context.h"

namespace Moment::Symmetrized {

    SymmetrizedContext::SymmetrizedContext(const Context& source_context)
        : Context(0), base_context{source_context} {

    }


}