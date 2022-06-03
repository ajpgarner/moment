/**
 * operator.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator.h"

#include <iostream>

std::ostream &NPATK::operator<<(std::ostream &os, const NPATK::Operator &op) {
    os << op.party.id << "_" << op.id;
    return os;
}