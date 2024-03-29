/**
 * locality_context.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "scenarios/context.h"
#include "party.h"

#include <memory>

namespace Moment::Locality {

    class LocalityOperatorFormatter;

    class LocalityContext : public Context {
    public:
        /**
         * Read-only range over parties in the context.
         */
        struct PartiesRange {
        private:
            LocalityContext& the_context;
        public:
            constexpr explicit PartiesRange(LocalityContext& context) noexcept : the_context{context} { }

            [[nodiscard]] auto begin() const noexcept { return the_context.parties.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_context.parties.cend(); }

            [[nodiscard]] const Party& operator[](size_t index) const noexcept {
                assert(index < the_context.parties.size());
                return the_context.parties[index];
            }

            [[nodiscard]] constexpr size_t size() const noexcept { return the_context.parties.size(); }

            [[nodiscard]] constexpr bool empty() const noexcept { return the_context.parties.empty(); }
        };

    public:
        PartiesRange Parties;

    private:
        std::vector<Party> parties;

        std::vector<party_name_t> global_mmt_id_to_party;
        std::vector<party_name_t> global_op_id_to_party;
        std::vector<PMOIndex> global_to_local_indices;

        size_t total_measurement_count = 0;
        std::vector<size_t> mmts_per_party;
        std::vector<size_t> ops_per_party;


    public:
        LocalityContext();

        explicit LocalityContext(std::vector<Party>&& parties) noexcept;

        /**
         * Use additional context to simplify an operator string.
         * @param op_sequence The string of operators
         * @return True if sequence is zero (cf. identity).
         */
        bool additional_simplification(sequence_storage_t &op_sequence, SequenceSignType& sign_type) const override;

        /** Converts global measurement index to Party, Measurement pair */
        [[nodiscard]] PMIndex global_index_to_PM(size_t global_index) const noexcept;

        /** Gets global measurement index from PMO index */
        [[nodiscard]] size_t get_global_mmt_index(const PMOIndex& pm_index) const noexcept {
            assert (pm_index.party < this->parties.size());
            const auto& party = this->parties[pm_index.party];
            assert (pm_index.mmt < party.measurements.size());
            return party.global_measurement_offset + pm_index.mmt;
        }

        /** Gets global measurement index from party and measurement*/
        [[nodiscard]] size_t get_global_mmt_index(party_name_t party_id, mmt_name_t mmt_id) const noexcept {
            assert (party_id < this->parties.size());
            const auto& party = this->parties[party_id];
            assert (mmt_id < party.measurements.size());
            return party.global_measurement_offset + mmt_id;
        }

        /** Populates global index from party & measurements in pm_index */
        void populate_global_mmt_index(std::span<PMIndex> pm_index) const noexcept;

        /** Creates vector of global measurement numbers based off PM index.
         * @throws std::range_error if any Party/Measurement in span is invalid.
         * */
        [[nodiscard]] std::vector<size_t> PM_to_global_index(std::span<const PMIndex> pm_index) const;

        /**
         * Returns total number of unique measurements
         */
        [[nodiscard]] size_t measurement_count() const noexcept { return this->total_measurement_count; }

        /**
         * Returns total number of measurements per party in context;
         */
        [[nodiscard]] const std::vector<size_t>& measurements_per_party() const noexcept {
            return this->mmts_per_party;
        }

        /**
         * Returns number of outcomes for each measurement referred to by indices
         * @throws std::range_error if any Party/Measurement in span is invalid.
         */
        [[nodiscard]] std::vector<size_t> outcomes_per_measurement(std::span<const PMIndex> indices) const;


        /**
         * Returns number of outcomes for every measurement in context.
         */
        [[nodiscard]] std::vector<size_t> outcomes_per_measurement() const;

        /**
         * Returns total number of operators per party in context.
         */
        [[nodiscard]] const std::vector<size_t>& operators_per_party() const noexcept {
            return this->ops_per_party;
        }

        /**
         * Gets the part associated with an operator.
         */
        [[nodiscard]] inline const Party& associated_party(oper_name_t oper) const {
            assert((oper >= 0) && (oper < this->size()));
            return this->parties[this->global_op_id_to_party[oper]];
        }


        /**
         * Returns number of outcomes per party in context.
         */
        [[nodiscard]] std::vector<size_t> outcomes_per_party() const;

        using Context::format_sequence;

        void format_sequence(ContextualOS &os, const OperatorSequence &seq) const override;

        /**
         * Generates a formatted string representation of a list of PMO indices
         */
        [[nodiscard]] std::string format_sequence(const LocalityOperatorFormatter& formatter,
                                                  std::span<const PMOIndex> indices, bool zero = false) const;

        /**
         * Summarize the context as a string.
         */
        [[nodiscard]] std::string to_string() const override;

    protected:
        std::unique_ptr<OperatorSequenceGenerator> new_osg(size_t word_length) const override;
    };

}