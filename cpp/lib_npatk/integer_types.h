/**
 * integer_types.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <cinttypes>
#include <cstddef>

namespace NPATK {

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
    using oper_name_t = int64_t;

    /**
     * For enumerating symbolic algebra elements
     */
    using symbol_name_t = int64_t;

};