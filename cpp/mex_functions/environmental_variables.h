/**
 * environmental_variables.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "multithreading/multithreading.h"

#include <iosfwd>
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

        Multithreading::MultiThreadPolicy mt_policy;

    public:
        EnvironmentalVariables();

        EnvironmentalVariables(const EnvironmentalVariables& reference);

        ~EnvironmentalVariables();

        void set_locality_formatter(std::shared_ptr<Locality::LocalityOperatorFormatter> replacement);

        [[nodiscard]] std::shared_ptr<Locality::LocalityOperatorFormatter> get_locality_formatter() const;

        inline void set_mt_policy(Multithreading::MultiThreadPolicy new_policy) {
            this->mt_policy = new_policy;
        }

        [[nodiscard]] inline Multithreading::MultiThreadPolicy get_mt_policy() const {
            return this->mt_policy;
        }


        friend std::ostream& operator<<(std::ostream& os, const EnvironmentalVariables& ev);

    };

}