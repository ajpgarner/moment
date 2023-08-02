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
    concept accepts_contextual_os = requires(const functor_t& f, ContextualOS& os) {
        {f(os)};
    };

    template<accepts_contextual_os functor_t>
    std::string make_contextualized_string(const Context& context,
                                           const SymbolTable& symbols,
                                           const functor_t& functor) {
        std::stringstream ss;
        ContextualOS cSS{ss, context, symbols};
        functor(cSS);
        return ss.str();
    }

    template<typename object_t>
    std::string make_contextualized_string(const StringFormatContext& sf_context,
                                           const object_t& object) {
        std::stringstream ss;
        ContextualOS cSS{ss, sf_context};
        cSS << object;
        return ss.str();
    }


    template<accepts_contextual_os functor_t>
    std::string make_contextualized_string(const StringFormatContext& sf_context,
                                           const functor_t& functor) {
        std::stringstream ss;
        ContextualOS cSS{ss, sf_context};
        functor(cSS);
        return ss.str();
    }


}