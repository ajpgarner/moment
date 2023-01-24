/**
 * environmental_variables.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <memory>

namespace Moment {
    namespace Locality {
        class LocalityOperatorFormatter;
    }
}

namespace Moment::mex {

    class EnvironmentalVariables {
    private:
        std::shared_ptr<Locality::LocalityOperatorFormatter> the_l_op_formatter;

    public:
        EnvironmentalVariables();

        ~EnvironmentalVariables();

        void set_locality_formatter(std::shared_ptr<Locality::LocalityOperatorFormatter> replacement);

        [[nodiscard]] std::shared_ptr<Locality::LocalityOperatorFormatter> get_locality_formatter() const;


    };

}