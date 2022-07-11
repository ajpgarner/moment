/**
 * setting.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "setting.h"

namespace NPATK::mex {
    namespace classes {

        Outcome::Outcome(matlab::engine::MATLABEngine &engine,
                         const size_t p_index, const size_t m_index, const size_t o_index,
                         matlab::data::Array& rawInput)
                : MATLABClass(engine, "Outcome", MATLABClass::FieldTypeMap{
                                                        {"Id", matlab::data::ArrayType::UINT64}
                                                 }, rawInput, o_index) {
            // Verify outcome object...
            std::stringstream errMsg;
            errMsg << "Invalid Outcome #" << (o_index + 1) << " (Party #" << (p_index +1) << ", "
                                          << "Measurement #" << (m_index+1) << "): ";

            const auto internal_index = this->property_scalar<uint64_t>("Id");
            if (o_index != (internal_index - 1)) {
                errMsg << "Internal index " << internal_index << " does not match order in list.";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }
        }


        Measurement::Measurement(matlab::engine::MATLABEngine &engine, const size_t party_index,
                                 const size_t mmt_index, matlab::data::Array& rawInput)
            : MATLABClass(engine, "Measurement", MATLABClass::FieldTypeMap{
                                                    {"Id", matlab::data::ArrayType::UINT64},
                                                    {"Name", matlab::data::ArrayType::MATLAB_STRING},
                                                    {"Outcomes", matlab::data::ArrayType::HANDLE_OBJECT_REF}
                                                  }, rawInput, mmt_index) {
            // Verify measurement object...
            std::stringstream errMsg;
            errMsg << "Invalid Measurement #" << (mmt_index + 1) << " (Party #" << (party_index +1) << "): ";

            const auto internal_index = this->property_scalar<uint64_t>("Id");
            if (mmt_index != (internal_index - 1)) {
                errMsg << "Internal index " << internal_index << " does not match order in list.";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            // Check outcome list is good...
            auto outcomeListArray = this->property("Outcomes");
            if (outcomeListArray.isEmpty()) {
                errMsg << "At least one outcome must be specified";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            auto outcomeDims = outcomeListArray.getDimensions();
            if (outcomeDims.size() != 2) {
                errMsg << "Invalid outcome list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }
            if (outcomeDims[0] != 1) {
                errMsg << "Invalid outcome list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }
            if (outcomeDims[1] <= 0) {
                errMsg << "Invalid outcome list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            // Now, parse outcomes...
            this->outcomeRaw = static_cast<matlab::data::ObjectArray>(outcomeListArray);
            const size_t outcome_count = this->outcomeRaw.getNumberOfElements();
            this->outcomes.reserve(outcome_count);
            for (size_t oIndex = 0; oIndex < outcome_count; ++oIndex) {
                this->outcomes.emplace_back(engine, party_index, mmt_index, oIndex, this->outcomeRaw);
            }
        }

        Party::Party(matlab::engine::MATLABEngine &engine, const size_t party_index, matlab::data::Array& rawInput)
                : MATLABClass(engine, "Party",
                              MATLABClass::FieldTypeMap{
                                      {"Id",           matlab::data::ArrayType::UINT64},
                                      {"Name",         matlab::data::ArrayType::MATLAB_STRING},
                                      {"RawOperators", matlab::data::ArrayType::UINT64},
                                      {"Measurements", matlab::data::ArrayType::HANDLE_OBJECT_REF}
                              }, rawInput, party_index) {

            // Verify party object...
            std::stringstream errMsg;
            errMsg << "Invalid Party #" << (party_index + 1) << ": ";

            const auto internal_index = this->property_scalar<uint64_t>("Id");
            if (party_index != (internal_index - 1)) {
                errMsg << "Internal index " << internal_index << " does not match order in list.";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            // Check measurement list is good...
            auto mmtListArray = this->property("Measurements");
            if (mmtListArray.isEmpty()) {
                errMsg << "At least one measurement must be specified";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            auto mmtDims = mmtListArray.getDimensions();
            if (mmtDims.size() != 2) {
                errMsg << "Invalid measurement list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }
            if (mmtDims[0] != 1) {
                errMsg << "Invalid measurement list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }
            if (mmtDims[1] <= 0) {
                errMsg << "Invalid measurement list (must be 1xN array).";
                throw errors::bad_class_exception{this->className, errMsg.str()};
            }

            // Now, parse measurements...
            this->mmtRaw = static_cast<matlab::data::ObjectArray>(mmtListArray);
            const size_t mmt_count = this->mmtRaw.getNumberOfElements();
            this->mmts.reserve(mmt_count);
            for (size_t mIndex = 0; mIndex < mmt_count; ++mIndex) {
                this->mmts.emplace_back(engine, party_index, mIndex, this->mmtRaw);
            }
        }

        Setting::Setting(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput)
                : MATLABClass(engine, "Setting",
                              MATLABClass::FieldTypeMap{
                                      {std::string("Parties"), matlab::data::ArrayType::HANDLE_OBJECT_REF}
                              }, std::move(rawInput)) {

            // Check party list is good...
            auto partyListArray = this->property("Parties");
            if (partyListArray.isEmpty()) {
                throw errors::bad_class_exception{this->className, "At least one Party must be specified in Setting."};
            }

            auto partyDims = partyListArray.getDimensions();
            if (partyDims.size() != 2) {
                throw errors::bad_class_exception{this->className, "Invalid Party list (must be 1xN array)."};
            }
            if (partyDims[0] != 1) {
                throw errors::bad_class_exception{this->className, "Invalid Party list (must be 1xN array)."};
            }
            if (partyDims[1] <= 0) {
                throw errors::bad_class_exception{this->className, "Invalid Party list (must be 1xN array)."};
            }

            // Now, parse parties...
            this->partyRaw = static_cast<matlab::data::ObjectArray>(partyListArray);

            const size_t party_count = partyRaw.getNumberOfElements();
            this->parties.reserve(party_count);
            for (size_t pIndex = 0; pIndex < party_count; ++pIndex) {
                this->parties.emplace_back(engine, pIndex, this->partyRaw);
            }

        }

        std::unique_ptr<Context> Setting::make_context() {
            size_t num_parties = this->Parties().size();
            std::vector<NPATK::Party> partyList{};
            partyList.reserve(num_parties);

            size_t party_index = 0;
            for (auto& party : this->Parties()) {

                const auto party_nameMLS = party.property_scalar<matlab::data::MATLABString>("Name");
                const std::string party_name{matlab::engine::convertUTF16StringToUTF8String(*party_nameMLS)};

                const auto raw_op_count = party.property_scalar<uint64_t>("RawOperators");

                NPATK::Party constructed_party{static_cast<party_name_t>(party_index),
                                               party_name, static_cast<oper_name_t>(raw_op_count)};

                for (auto& mmt : party.Measurements()) {

                    const auto mmt_nameMLS = mmt.property_scalar<matlab::data::MATLABString>("Name");
                    const std::string mmt_name{matlab::engine::convertUTF16StringToUTF8String(*mmt_nameMLS)};
                    const uint64_t mmt_outcomes = mmt.Outcomes().size();
                    constructed_party.add_measurement(NPATK::Measurement{mmt_name, mmt_outcomes});
                }

                partyList.emplace_back(std::move(constructed_party));
                ++party_index;
            }

            // Create context from party list...
            return std::make_unique<Context>(std::move(partyList));
        }

    }

    std::pair<std::unique_ptr<classes::Setting>, std::optional<std::string>>
    read_as_setting(matlab::engine::MATLABEngine &engine,
                    matlab::data::Array raw_data) {

        // Check just one
        if (raw_data.getNumberOfElements() != 1) {
            return {nullptr, std::string("Only one Setting object should be supplied.")};
        }

        // Check object is an instance of 'Setting'
        std::unique_ptr<classes::Setting> ptrSetting;
        try {
            ptrSetting = std::make_unique<classes::Setting>(engine, std::move(raw_data));
        } catch (const errors::bad_class_exception &bce) {
            return {nullptr, std::string{bce.what()}};
        }

        return std::make_pair(std::move(ptrSetting), std::nullopt);
    }
}
