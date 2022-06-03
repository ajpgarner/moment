/**
 * operator_collection.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "operator.h"

#include <cassert>

#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace NPATK {

    class PartyInfo : public Party {
    public:
        const std::string name;
    private:
        size_t global_offset = 0;
        std::vector<Operator> operators;
        std::set<std::pair<oper_name_t, oper_name_t>> mutex;

    public:
        PartyInfo(party_name_t id, std::string named, oper_name_t num_opers,
                  size_t global_offset = 0, Operator::Flags default_flags = Operator::Flags::None);

        explicit PartyInfo(party_name_t id, oper_name_t num_opers,
                           size_t global_offset = 0, Operator::Flags default_flags = Operator::Flags::None)
            : PartyInfo(id, std::to_string(id), num_opers, global_offset, default_flags) { }

        [[nodiscard]] constexpr auto begin() const noexcept { return this->operators.begin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return this->operators.end(); }

        constexpr Operator& operator[](size_t index)  noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }

        constexpr const Operator& operator[](size_t index) const noexcept {
            assert(index < operators.size());
            return this->operators[index];
        }

        void add_mutex(oper_name_t lhs_id, oper_name_t rhs_id) {
            if (lhs_id < rhs_id) {
                this->mutex.emplace(std::make_pair(lhs_id, rhs_id));
            } else {
                this->mutex.emplace(std::make_pair(rhs_id, lhs_id));
            }
        }

        [[nodiscard]] constexpr bool exclusive(oper_name_t lhs_id, oper_name_t rhs_id) const noexcept {
            return mutex.contains((lhs_id < rhs_id) ? std::make_pair(lhs_id, rhs_id) : std::make_pair(rhs_id, lhs_id));
        };

        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        [[nodiscard]] constexpr size_t offset() const noexcept { return this->global_offset; }
    };

    class OperatorCollection {
    public:
        struct PartiesRange {
        private:
            OperatorCollection& the_gen;
        public:
            constexpr explicit PartiesRange(OperatorCollection& npag) noexcept : the_gen{npag} { }

            [[nodiscard]] auto begin() noexcept { return the_gen.parties.begin(); }
            [[nodiscard]] auto begin() const noexcept { return the_gen.parties.cbegin(); }
            [[nodiscard]] auto end() noexcept { return the_gen.parties.end(); }
            [[nodiscard]] auto end() const noexcept { return the_gen.parties.cend(); }

            [[nodiscard]] PartyInfo& operator[](size_t index) noexcept {
                assert(index < the_gen.parties.size());
                return the_gen.parties[index];
            }
            [[nodiscard]] const PartyInfo& operator[](size_t index) const noexcept {
                assert(index < the_gen.parties.size());
                return the_gen.parties[index];
            }

            [[nodiscard]] size_t size() const noexcept { return the_gen.parties.size(); }

            [[nodiscard]] bool empty() const noexcept { return the_gen.parties.empty(); }
        };

        /**
         * Iterates over every symbol in every party.
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

            AllOperatorConstIterator(const OperatorCollection& generator, bool)
                : party_iter{generator.parties.end()}, party_iter_end{generator.parties.end()} { }

            friend class OperatorCollection;

        public:
            explicit AllOperatorConstIterator(const OperatorCollection& generator)
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

        explicit OperatorCollection(std::vector<PartyInfo>&& parties) noexcept;

        OperatorCollection(std::initializer_list<oper_name_t> oper_per_party_list,
                           Operator::Flags default_flags = Operator::Flags::None)
            : OperatorCollection(OperatorCollection::make_party_list(oper_per_party_list, default_flags)) { }

        OperatorCollection(party_name_t num_parties, oper_name_t opers_per_party,
                           Operator::Flags default_flags = Operator::Flags::None)
            : OperatorCollection(OperatorCollection::make_party_list(num_parties, opers_per_party, default_flags)) { }

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

    private:
        static std::vector<PartyInfo> make_party_list(party_name_t num_parties, oper_name_t opers_per_party,
                                                      Operator::Flags default_flags);
        static std::vector<PartyInfo> make_party_list(std::initializer_list<oper_name_t> oper_per_party_list,
                                                      Operator::Flags default_flags);

    };
}