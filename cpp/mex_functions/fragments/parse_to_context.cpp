/**
 * parse_to_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "parse_to_context.h"
#include "utilities/reporting.h"
#include "utilities/verify_as_class.h"

#include "cppmex/mexMatlabEngine.hpp"
#include "cppmex/mexException.hpp"

namespace NPATK::mex {

    std::pair<bool, std::optional<std::string>> verify_as_setting(matlab::engine::MATLABEngine& engine,
            const matlab::data::Array& raw_data) {

        // Check just one
        if (raw_data.getNumberOfElements() != 1) {
            return {false, std::string("Only one Setting object should be supplied.")};
        }

        // Check object declares itself to be an instance of 'Setting'
        auto [isSettingObj, whyNot] = verify_as_class_handle(engine, raw_data, "Setting");
        if (!isSettingObj) {
            return {false, whyNot};
        }

        // Check parties exist...
        auto maybeParties = try_get_property(engine, raw_data, "Parties");
        if (!maybeParties.has_value()) {
            return {false, std::string("Setting object should contain property 'Parties'.")};
        }
        const auto& partyList = maybeParties.value();

        // Check type of parties... make sure is a list.
        auto [isPartyList, whyNotPL] = verify_as_class_handle(engine, partyList, "Party");
        if (!isPartyList) {
            return {false, std::string("Parties property of Setting object was not a list of Party class objects.")};
        }
        if (partyList.isEmpty()) {
            return {false, std::string("At least one Party must be specified in Setting.")};
        }

        auto partyDims = partyList.getDimensions();
        if (partyDims.size() != 2) {
            return {false, std::string("Invalid Party list (must be 1xN array).")};
        }
        if (partyDims[0] != 1) {
            return {false, std::string("Invalid Party list (must be 1xN array).")};
        }
        if (partyDims[1] <= 0) {
            return {false, std::string("Invalid Party list (must be 1xN array).a")};
        }
        size_t num_parties = partyDims[1];

        // Now, verify each party object in turn...
        for (size_t party_index = 0; party_index < num_parties; ++party_index) {
            std::stringstream errMsg;
            errMsg << "Invalid Party #" << (party_index+1) << ": ";

            auto idObj = try_get_property(engine, partyList, party_index, "Id");
            if (!idObj.has_value() || idObj.value().isEmpty()) {
                errMsg << "Missing Id";
                return {false, errMsg.str()};
            }
            if (idObj.value().getType() != matlab::data::ArrayType::UINT64) {
                errMsg << "Id should be uint64";
                return {false, errMsg.str()};
            }

            const uint64_t internal_index = *(static_cast<matlab::data::TypedArray<uint64_t>>(*idObj).begin());
            if (party_index != (internal_index-1)) {
                errMsg << "Internal index " << internal_index << " does not match order in list.";
                return {false, errMsg.str()};
            }

            auto nameObj = try_get_property(engine, partyList, party_index, "Name");
            if (!nameObj.has_value() || nameObj.value().isEmpty()) {
                errMsg << "Missing Name";
                return {false, errMsg.str()};
            }
            if ((nameObj.value().getType() != matlab::data::ArrayType::MATLAB_STRING)) {
                errMsg << "Name should be String";
                return {false, errMsg.str()};
            }

            auto rawOpObj = try_get_property(engine, partyList, party_index, "RawOperators");
            if (!rawOpObj.has_value() || rawOpObj.value().isEmpty()) {
                errMsg << "Missing RawOperators";
                return {false, errMsg.str()};
            }
            if (rawOpObj.value().getType() != matlab::data::ArrayType::UINT64) {
                errMsg << "RawOperators should be uint64";
                return {false, errMsg.str()};
            }

            auto measurementsObj = try_get_property(engine, partyList, party_index, "Measurements");
            if (!measurementsObj.has_value()) {
                errMsg << "Missing Measurements";
                return {false, errMsg.str()};
            }

            const auto& mmtListArray = measurementsObj.value();
            auto [isMmtStruct, whyNotMmtStruct] = verify_struct(engine, mmtListArray, {"name", "num_outcomes"});
            if (!isMmtStruct) {
                errMsg << "Invalid Measurements: " << whyNotMmtStruct.value();
                return {false, errMsg.str()};
            }
            auto mmtListStruct = static_cast<matlab::data::StructArray>(mmtListArray);
            if (mmtListStruct.isEmpty()) {
                errMsg << "No measurements provided.";
                return {false, errMsg.str()};
            } else {
                if (mmtListStruct[0]["name"].getType() != matlab::data::ArrayType::MATLAB_STRING) {
                    errMsg << "'name' field of Measurements must be String.";
                    return {false, errMsg.str()};
                }
                if (mmtListStruct[0]["num_outcomes"].getType() != matlab::data::ArrayType::UINT64) {
                    errMsg << "'num_outcomes' field of Measurements must be uint64.";
                    return {false, errMsg.str()};
                }
            }
        }

        // Okay, all good...
        return {true, {}};
    }

    std::unique_ptr<Context> parse_to_context(matlab::engine::MATLABEngine& engine,
                                              matlab::data::Array& raw_data) {

        const auto partiesObj = *try_get_property(engine, raw_data, "Parties");

        size_t num_parties = partiesObj.getNumberOfElements();
        std::vector<PartyInfo> partyList{};
        partyList.reserve(num_parties);

        for (size_t party_index = 0; party_index < num_parties; ++party_index) {
            auto rawIdArr = get_property<uint64_t>(engine, partiesObj, party_index, "Id");
            const uint64_t internal_index = *rawIdArr.begin();
            if (party_index != (internal_index-1)) {
                throw_error(engine, errors::internal_error, "Party ID mismatch!!");
            }

            auto nameArr = get_property<matlab::data::MATLABString>(engine, partiesObj, party_index, "Name");
            const std::string party_name{matlab::engine::convertUTF16StringToUTF8String((*nameArr.begin()))};

            auto rawOperArr = get_property<uint64_t>(engine, partiesObj, party_index, "RawOperators");
            const uint64_t raw_op_count = *rawOperArr.begin();

            PartyInfo party{static_cast<party_name_t>(party_index), party_name, static_cast<oper_name_t>(raw_op_count)};

            auto mmtArr = get_property_struct(engine, partiesObj, party_index, "Measurements");
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