/**
 * integer_types.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cinttypes>
#include <cstddef>

namespace Moment {

    /**
     * For enumerating parties (Alice, Bob, etc).
     */
    using party_name_t = int16_t;

    /**
     * For enumerating measurements of parties
     */
    using mmt_name_t = int16_t;

    /**
     * For enumerating operators.
     */
    using oper_name_t = int16_t;

    /**
     * For enumerating symbolic algebra elements
     */
    using symbol_name_t = int64_t;

    /**
     * The maximum length of operator sequence before heap allocations are required
     */
    constexpr size_t op_seq_stack_length = 16ULL;

};