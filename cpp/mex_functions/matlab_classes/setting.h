/**
 * setting.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "matlab_class.h"
#include "operators/context.h"

#include <memory>

namespace NPATK::mex {

    namespace classes {
        class Party : public MATLABClass {
        public:
            Party(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput);

            size_t num_parties = 0;
        };

        class Setting : public MATLABClass {
        private:
            std::unique_ptr<Party> partyListPtr;

        public:
            Setting(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput);

            constexpr Party &Parties() noexcept {
                return *partyListPtr;
            }

            std::unique_ptr<Context> make_context();
        };
    }

    /**
     * Check if supplied array object matches a valid specification of (matlab) 'Setting' class.
     * @param engine Handle to MATLAB engine.
     * @param raw_data The raw object to verify.
     * @return Pair: first: true ptr to Setting object, if valid; second: why verification failed, set when first is null.
     */
    [[nodiscard]] std::pair<std::unique_ptr<classes::Setting>, std::optional<std::string>>
    read_as_setting(matlab::engine::MATLABEngine& engine, matlab::data::Array raw_data);

}