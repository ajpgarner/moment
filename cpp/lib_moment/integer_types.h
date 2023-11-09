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

#ifdef MOMENT_DEBUG
    /** True: moment will be compiled with some additional debug checks. */
    constexpr const bool debug_mode = true;
#else
    /** False: moment will be compiled without additional debug checks. */
    constexpr const bool debug_mode = false;
#endif

    using std::size_t;
    using std::ptrdiff_t;

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
    using symbol_name_t = int32_t;

    /**
     * The maximum length of operator sequence before heap allocations are required
     */
    constexpr size_t op_seq_stack_length = 16ULL;

};