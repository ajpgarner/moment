/**
 * locality_osg.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "dictionary/operator_sequence_generator.h"

namespace Moment::Locality {

    class Party;
    class LocalityContext;

    class LocalityOperatorSequenceGenerator : public OperatorSequenceGenerator {
    public:
        class PartyOSG {
        public:
            const class Party * party;
        protected:
            std::vector<OperatorSequence> unique_sequences;
            std::vector<ptrdiff_t> word_length_boundaries;

        public:
            explicit PartyOSG(const LocalityContext& context, const Party* party, size_t max_word_length);

            /**
             * Get range of operator sequences of requested length.
             */
            [[nodiscard]] std::span<const OperatorSequence> operator[](size_t word_length) const noexcept;

            /**
             * Get range over all operator sequences.
             */
            [[nodiscard]] inline std::span<const OperatorSequence> all() const noexcept {
                return {this->unique_sequences.cbegin(), this->unique_sequences.size()};
            }

            /**
             * Get maximum word length encoded
             */
            [[nodiscard]] inline size_t word_length() const noexcept { return this->word_length_boundaries.size()-1; }

        };

    protected:
        const LocalityContext& localityContext;

        std::vector<PartyOSG> parties;

    public:
        LocalityOperatorSequenceGenerator(const LocalityContext& context, size_t word_length);

        [[nodiscard]] inline const PartyOSG& Party(size_t idx) const noexcept { return this->parties[idx]; }

        [[nodiscard]] inline size_t PartyCount() const noexcept { return this->parties.size(); }

    private:
        void populate_zero_parties();

        void populate_one_party();

        void populate_general();
    };

}