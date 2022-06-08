/**
 * operator_collection.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"
#include "party_info.h"

#include <cassert>

#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace NPATK {

    class OperatorSequence;

    class Context {
    public:
        /**
         * Range over parties in the context.
         */
        struct PartiesRange {
        private:
            Context& the_context;
        public:
            constexpr explicit PartiesRange(Context& context) noexcept : the_context{context} { }

            [[nodiscard]] auto begin() noexcept { return the_context.parties.begin(); }
            [[nodiscard]] auto begin() const noexcept { return the_context.parties.cbegin(); }
            [[nodiscard]] auto end() noexcept { return the_context.parties.end(); }
            [[nodiscard]] auto end() const noexcept { return the_context.parties.cend(); }

            [[nodiscard]] PartyInfo& operator[](size_t index) noexcept {
                assert(index < the_context.parties.size());
                return the_context.parties[index];
            }
            [[nodiscard]] const PartyInfo& operator[](size_t index) const noexcept {
                assert(index < the_context.parties.size());
                return the_context.parties[index];
            }

            [[nodiscard]] size_t size() const noexcept { return the_context.parties.size(); }

            [[nodiscard]] bool empty() const noexcept { return the_context.parties.empty(); }
        };

        /**
         * Iterates over every operator in every party.
         */
        class AllOperatorConstIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = Operator;

        private:
            std::vector<PartyInfo>::const_iterator party_iter;
            std::vector<PartyInfo>::const_iterator party_iter_end;
            std::vector<Operator>::const_iterator oper_iter;

            AllOperatorConstIterator(const Context& generator, bool)
                : party_iter{generator.parties.end()}, party_iter_end{generator.parties.end()} { }

            friend class Context;

        public:
            explicit AllOperatorConstIterator(const Context& generator)
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

            bool operator==(const AllOperatorConstIterator& rhs) const noexcept  {
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

            AllOperatorConstIterator& operator++() noexcept {
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

    private:
        std::vector<PartyInfo> parties;
        size_t total_operator_count = 0;

    public:
        PartiesRange Parties;

        explicit Context(std::vector<PartyInfo>&& parties) noexcept;

        Context(std::initializer_list<oper_name_t> oper_per_party_list,
                Operator::Flags default_flags = Operator::Flags::None)
            : Context(Context::make_party_list(oper_per_party_list, default_flags)) { }

        Context(party_name_t num_parties, oper_name_t opers_per_party,
                Operator::Flags default_flags = Operator::Flags::None)
            : Context(Context::make_party_list(num_parties, opers_per_party, default_flags)) { }

        [[nodiscard]] auto begin() const noexcept {
            return AllOperatorConstIterator{*this};
        }

        [[nodiscard]] auto end() const noexcept {
            return AllOperatorConstIterator{*this, true};
        }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->total_operator_count; }

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


    private:
        static std::vector<PartyInfo> make_party_list(party_name_t num_parties, oper_name_t opers_per_party,
                                                      Operator::Flags default_flags);
        static std::vector<PartyInfo> make_party_list(std::initializer_list<oper_name_t> oper_per_party_list,
                                                      Operator::Flags default_flags);

    };
}