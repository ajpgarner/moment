/**
 * context.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "integer_types.h"

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
        using oper_iter_t = std::vector<oper_name_t>::const_iterator;

    protected:
        std::vector<oper_name_t> operators;

    public:
        constexpr Context() = default;

        explicit Context(size_t operator_count);

        virtual ~Context() = default;

        /** Iterate over every operator */
        [[nodiscard]] constexpr auto begin() const noexcept {
            return this->operators.cbegin();
        }

        /** End of iteration over every operator */
        [[nodiscard]] constexpr auto end() const noexcept {
            return this->operators.cend();
        }

        /** Gets total number of operators in Context */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        /** True, if no operators in Context */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        /** Get operator in context by index */
        [[nodiscard]] constexpr oper_name_t operator[](size_t index) const noexcept {
            assert(index < this->operators.size());
            return this->operators[index];
        }

        /**
         * Use additional context to simplify an operator string.
         * @param op_sequence The string of operators
         * @return True if sequence is zero (cf. identity).
         */
         virtual bool additional_simplification(std::vector<oper_name_t>& op_sequence) const;

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
          * Calculates a non-colliding hash (i.e. unique number) for a particular operator sequence.
          * The hash is in general dependent on the total number of distinct operators in the context.
          * The zero operator is guaranteed a hash of 0.
          * The identity operator is guaranteed a hash of 1.
          * @param seq The raw operator sequence to calculate the hash of.
          * @return An integer hash.
          */
         [[nodiscard]] size_t hash(const std::vector<oper_name_t>& rawSeq) const noexcept;

         /**
          * Generates a formatted string representation of an operator sequence
          */
          [[nodiscard]] virtual std::string format_sequence(const OperatorSequence& seq) const;

         /**
          * Summarize the context as a string.
          */
         [[nodiscard]] virtual std::string to_string() const;

    public:


         friend std::ostream& operator<< (std::ostream& os, const Context& context);

    };

}