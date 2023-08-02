/**
 * context_os_helper.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "contextual_os.h"

#include <sstream>
#include <string>

namespace Moment {

    template<typename functor_t>
    std::string make_contextualized_string(const Context& context,
                                           const SymbolTable& symbols,
                                           functor_t functor) {
        std::stringstream ss;
        ContextualOS cSS{ss, context, symbols};
        functor(cSS);
        return ss.str();
    }

    template<typename functor_t>
    std::string make_contextualized_string(const StringFormatContext& sf_context,
                                           functor_t functor) {
        std::stringstream ss;
        ContextualOS cSS{ss, sf_context};
        functor(cSS);
        return ss.str();
    }
}