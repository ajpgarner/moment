/**
 * context.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"
#include "party.h"

#include <cassert>

#include <iterator>
#include <iosfwd>
#include <set>
#include <string>
#include <span>
#include <vector>

namespace NPATK {

    class OperatorSequence;

    class Context {
    public:
        /**
         * Read-only range over parties in the context.
         */
        struct PartiesRange {
        private:
            Context& the_context;
        public:
            constexpr explicit PartiesRange(Context& context) noexcept : the_context{context} { }

            [[nodiscard]] auto begin() const noexcept { return the_context.parties.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_context.parties.cend(); }

            [[nodiscard]] const Party& operator[](size_t index) const noexcept {
                assert(index < the_context.parties.size());
                return the_context.parties[index];
            }

            [[nodiscard]] constexpr size_t size() const noexcept { return the_context.parties.size(); }

            [[nodiscard]] constexpr bool empty() const noexcept { return the_context.parties.empty(); }
        };

        /**
         * Read-only iterator over every operator in every party.
         */
        class AllOperatorConstIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = Operator;

        private:
            std::vector<Party>::const_iterator party_iter;
            std::vector<Party>::const_iterator party_iter_end;
            std::vector<Operator>::const_iterator oper_iter;

            constexpr AllOperatorConstIterator(const Context& generator, bool)
                : party_iter{generator.parties.end()}, party_iter_end{generator.parties.end()} { }

            friend class Context;

        public:
            constexpr explicit AllOperatorConstIterator(const Context& generator)
                : party_iter{generator.parties.begin()}, party_iter_end{generator.parties.end()} {
                if (party_iter != party_iter_end) {
                    oper_iter = party_iter->begin();
                }
            }

            const Operator& operator*() const noexcept {
                return *oper_iter;
            }

            const Operator * operator->() const noexcept {
                return oper_iter.operator->();
            }

            constexpr bool operator==(const AllOperatorConstIterator& rhs) const noexcept  {
                assert(this->party_iter_end == rhs.party_iter_end);

                // Not equivalent if not on same party.
                if (this->party_iter != rhs.party_iter) {
                    return false;
                }

                // On same party, and parties are at an end: equivalent.
                if (this->party_iter == this->party_iter_end) {
                    return true;
                }

                // Not at end, but on same party; compare party iterators.
                return (this->oper_iter == rhs.oper_iter);
            }

            constexpr AllOperatorConstIterator& operator++() noexcept {
                ++oper_iter;
                if (oper_iter == party_iter->end()) {
                    ++party_iter;
                    if (party_iter != party_iter_end) {
                        oper_iter = party_iter->begin();
                    }
                }
                return *this;
            }

            AllOperatorConstIterator operator++(int) & noexcept {
                AllOperatorConstIterator tmp{*this};
                ++(*this);
                return tmp;
            }
        };
        static_assert(std::input_iterator<AllOperatorConstIterator>);

    public:
        PartiesRange Parties;

    private:
        std::vector<Party> parties;
        std::vector<party_name_t> global_to_party;
        size_t total_measurement_count = 0;
        size_t total_operator_count = 0;

    public:
        constexpr Context() : parties{}, Parties{*this} { }

        explicit Context(std::vector<Party>&& parties) noexcept;

        /** Iterate over every operator in every party */
        [[nodiscard]] constexpr auto begin() const noexcept {
            return AllOperatorConstIterator{*this};
        }

        /** End of iteration over every operator in every party */
        [[nodiscard]] constexpr auto end() const noexcept {
            return AllOperatorConstIterator{*this, true};
        }

        /** Gets total number of operators in Context */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->total_operator_count; }

        /** True if, in principle, (part of), the context can be referred to by its Collins-Gisin form.
         * This would be the case if every party defines at least one measurement.
         */
        [[nodiscard]] bool admits_cg_form() const noexcept;

        /** Converts global measurement index to Party, Measurement pair */
        [[nodiscard]] PMIndex global_index_to_PM(size_t global_index) const noexcept;

        /** Gets global measurement index from PMO index */
        [[nodiscard]] size_t get_global_mmt_index(const PMOIndex& pm_index) const noexcept {
            assert (pm_index.party < this->parties.size());
            const auto& party = this->parties[pm_index.party];
            assert (pm_index.mmt < party.measurements.size());
            return party.global_mmt_offset + pm_index.mmt;
        }

        /** Populates global index from party & measurements in pm_index */
        void get_global_mmt_index(std::vector<PMIndex>& pm_index) const noexcept;

        /**
         * Adds party to context. Warning: might invalidate references/pointers to previous parties.
         * @param info The new party to add.
         */
        void add_party(Party info);

        /**
         * Use additional context to simplify operator string.
         * @param start The start of the string of operators
         * @param end  The end of the string of operators
         * @return The new end of the string of operators (could be different from end, if operators are removed)
         */
         [[nodiscard]] std::pair<std::vector<Operator>::iterator, bool>
         additional_simplification(std::vector<Operator>::iterator start,
                                  std::vector<Operator>::iterator end) const noexcept;

         /**
          * Calculates a non-colliding hash (i.e. unique number) for a particular operator sequence.
          * The hash is in general dependent on the total number of distinct operators in the context.
          * The zero operator is guaranteed a hash of 0.
          * The identity operator is guaranteed a hash of 1.
          * @param seq The operator sequence to calculate the hash of.
          * @return An integer hash.
          */
         [[nodiscard]] size_t hash(const OperatorSequence& seq) const noexcept;

         /**
          * Generates a formatted string representation of an operator sequence
          */
          [[nodiscard]] std::string format_sequence(const OperatorSequence& seq) const;

         /**
          * Generates a formatted string representation of a list of PMO indices
          */
          [[nodiscard]] std::string format_sequence(std::span<const PMOIndex> indices, bool zero = false) const;

          /**
           * Returns total number of unique measurements
           */
           [[nodiscard]] size_t measurement_count() const noexcept { return this->total_measurement_count; }

          /**
           * Returns total number of measurements per party in context;
           */
           [[nodiscard]] std::vector<size_t> measurements_per_party() const;

           /**
            * Returns total number of operators per party in context;
            */
            [[nodiscard]] std::vector<size_t> operators_per_party() const;


          /**
           * Recalculate global offsets (e.g., because a Party added measurements or operators).
           */
           void reenumerate();

         friend std::ostream& operator<< (std::ostream& os, const Context& context);

    };

}