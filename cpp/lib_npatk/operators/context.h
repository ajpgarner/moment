/**
 * context.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "integer_types.h"
#include "hashed_sequence.h"
#include "shortlex_hasher.h"

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
        protected:
        oper_name_t operator_count;

        ShortlexHasher hasher;

    public:
        explicit Context(size_t operator_count);

        virtual ~Context() = default;

        /** Gets total number of operators in Context */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operator_count; }

        /** True, if no operators in Context */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->operator_count == 0; }

        /**
         * Use context to simplify an operator string.
         * @param op_sequence The string of operators
         * @return True if sequence is zero (cf. identity).
         */
         virtual bool additional_simplification(std::vector<oper_name_t>& op_sequence, bool& negate) const;

         /**
          * Use context to simplify or substitute an operator sequence, at the point where it is taken as a moment.
          * @param seq The operator sequence to simplify
          */
         virtual OperatorSequence simplify_as_moment(OperatorSequence&& seq) const;

         /**
          * Does context know anything extra known about operator sequence X that would imply Re(X)=0 or Im(X)=0?
          * @param seq The operator sequence X to test.
          * @return Pair, first: true if real part is zero, second: true if imaginary part is zero.
          */
         [[nodiscard]] virtual std::pair<bool, bool> is_sequence_null(const OperatorSequence& seq) const noexcept {
             return {false, false};
         }

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
         [[nodiscard]] size_t hash(const std::vector<oper_name_t>& rawSeq) const noexcept {
             return this->hasher(rawSeq);
         }

         /**
          * Get handle to the hasher
          */
          const ShortlexHasher& the_hasher() const noexcept {
              return this->hasher;
          }

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