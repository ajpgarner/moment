/**
 * locality_collins_gisin.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "scenarios/collins_gisin.h"

namespace Moment::Locality {

    class LocalityContext;
    class LocalityMatrixSystem;

    class LocalityCollinsGisin : public CollinsGisin {
    public:
        const LocalityContext& context;

    public:
        explicit LocalityCollinsGisin(const LocalityMatrixSystem& lms);

    };
}