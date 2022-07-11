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

        class Outcome : public MATLABClass {
        public:
            Outcome(matlab::engine::MATLABEngine &engine, size_t p_index, size_t m_index, size_t o_index,
                    matlab::data::Array& rawInput);
        };

        class Measurement : public MATLABClass {
        private:
            matlab::data::Array outcomeRaw;
            std::vector<NPATK::mex::classes::Outcome> outcomes;

        public:
            Measurement(matlab::engine::MATLABEngine &engine, size_t p_index, size_t m_index,
                        matlab::data::Array& rawInput);

            constexpr auto& Outcomes() noexcept {
                return this->outcomes;
            }
        };

        class Party : public MATLABClass {
        private:
            matlab::data::Array mmtRaw;
            std::vector<NPATK::mex::classes::Measurement> mmts;

        public:
            Party(matlab::engine::MATLABEngine &engine, size_t index, matlab::data::Array& rawInput);

            constexpr auto& Measurements() noexcept {
                return this->mmts;
            }
        };

        class Setting : public MATLABClass {
        private:
            matlab::data::Array partyRaw;
            std::vector<NPATK::mex::classes::Party> parties;

        public:
            Setting(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput);

            constexpr auto &Parties() noexcept {
                return this->parties;
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