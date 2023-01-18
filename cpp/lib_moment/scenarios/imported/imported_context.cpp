/**
 * imported_context.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "imported_context.h"

namespace Moment::Imported {

    ImportedContext::ImportedContext(const bool is_real) : Context(0), purely_real{is_real} {

    }

    bool ImportedContext::can_be_nonhermitian() const noexcept {
        return !this->purely_real;
    }
}