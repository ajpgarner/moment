/**
 * setting.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "setting.h"

namespace NPATK::mex {
    namespace classes {

        Party::Party(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput)
                : MATLABClass(engine, "Party",
                              MATLABClass::FieldTypeMap{
                                      {"Id",           matlab::data::ArrayType::UINT64},
                                      {"Name",         matlab::data::ArrayType::MATLAB_STRING},
                                      {"RawOperators", matlab::data::ArrayType::UINT64},
                                      {"Measurements", matlab::data::ArrayType::STRUCT}
                              }, std::move(rawInput)) {

            this->num_parties = this->raw_data.getNumberOfElements();

            // Now, verify each party object in turn...
            for (size_t party_index = 0; party_index < num_parties; ++party_index) {
                std::stringstream errMsg;
                errMsg << "Invalid Party #" << (party_index + 1) << ": ";

                const auto internal_index = this->property_scalar<uint64_t>(party_index, "Id");
                if (party_index != (internal_index - 1)) {
                    errMsg << "Internal index " << internal_index << " does not match order in list.";
                    throw errors::bad_class_exception{this->className, errMsg.str()};
                }

                auto mmtListArray = this->property(party_index, "Measurements");
                auto [isMmtStruct, whyNotMmtStruct] = classes::verify_struct(engine, mmtListArray,
                                                                             {"name", "num_outcomes"});
                if (!isMmtStruct) {
                    errMsg << "Invalid Measurements: " << whyNotMmtStruct.value();
                    throw errors::bad_class_exception{this->className, errMsg.str()};
                }

                auto mmtListStruct = static_cast<matlab::data::StructArray>(mmtListArray);
                if (mmtListStruct.isEmpty()) {
                    errMsg << "No measurements provided in Measurements list.";
                    throw errors::bad_class_exception{this->className, errMsg.str()};
                } else {
                    if (mmtListStruct[0]["name"].getType() != matlab::data::ArrayType::MATLAB_STRING) {
                        errMsg << "'name' field of Measurements must be String.";
                        throw errors::bad_class_exception{this->className, errMsg.str()};
                    }
                    if (mmtListStruct[0]["num_outcomes"].getType() != matlab::data::ArrayType::UINT64) {
                        errMsg << "'num_outcomes' field of Measurements must be uint64.";
                        throw errors::bad_class_exception{this->className, errMsg.str()};
                    }
                }
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
            this->partyListPtr = std::make_unique<Party>(this->engine, std::move(partyListArray));
            if (!this->partyListPtr) {
                throw_error(engine, errors::internal_error, "Could not create handle to ObjectArray of Parties.");
            }

        }

        std::unique_ptr<Context> Setting::make_context() {
            assert(this->partyListPtr);

            size_t num_parties = this->Parties().num_parties;
            std::vector<NPATK::Party> partyList{};
            partyList.reserve(num_parties);

            for (size_t party_index = 0; party_index < num_parties; ++party_index) {
                //const auto internal_index = this->property_scalar<uint64_t>("Id");
                const auto nameMLS = this->partyListPtr->property_scalar<matlab::data::MATLABString>(party_index, "Name");
                const std::string party_name{matlab::engine::convertUTF16StringToUTF8String(*nameMLS)};

                const auto raw_op_count = this->partyListPtr->property_scalar<uint64_t>(party_index, "RawOperators");

                NPATK::Party party{static_cast<party_name_t>(party_index),
                            party_name,
                            static_cast<oper_name_t>(raw_op_count)};

                auto mmtArr = this->partyListPtr->property_struct(party_index, "Measurements");
                const size_t num_mmts = mmtArr.getNumberOfElements();
                for (size_t mmt_index = 0; mmt_index < num_mmts; ++mmt_index) {
                    const matlab::data::TypedArrayRef<matlab::data::MATLABString> mmtNameArr{mmtArr[mmt_index]["name"]};
                    const std::string mmt_name{matlab::engine::convertUTF16StringToUTF8String((*mmtNameArr.begin()))};

                    const matlab::data::TypedArrayRef<uint64_t> mmtNumOutcomeArr{mmtArr[mmt_index]["num_outcomes"]};
                    const uint64_t mmt_outcomes = *mmtNumOutcomeArr.begin();
                    party.add_measurement(Measurement{mmt_name, mmt_outcomes});
                }
                partyList.emplace_back(std::move(party));
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
