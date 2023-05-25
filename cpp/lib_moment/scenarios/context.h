/**
 * context.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "integer_types.h"
#include "hashed_sequence.h"
#include "operator_sequence.h"
#include "utilities/shortlex_hasher.h"

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace Moment {

    class OperatorSequence;
    class OperatorSequenceGenerator;
    class WordList;

    class Context {
    protected:
        oper_name_t operator_count;

        ShortlexHasher hasher;

    private:
        /** List of operator-sequence-generators */
        std::unique_ptr<WordList> word_list;

    public:
        explicit Context(size_t operator_count);

        virtual ~Context();

        /** Gets total number of operators in Context */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operator_count; }

        /** True, if no operators in Context */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->operator_count == 0; }

        /** True, if Context can generate (in principle) non-Hermitian operator strings */
        [[nodiscard]] virtual bool can_be_nonhermitian() const noexcept { return true; }

        /**
         * Use context to simplify an operator string.
         * @param op_sequence The string of operators
         * @return True if sequence is zero (cf. identity).
         */
         virtual bool additional_simplification(sequence_storage_t &op_sequence, bool& negate) const;

         /**
          * Use context to simplify or substitute an operator sequence, at the point where it is taken as a moment.
          * @param seq The operator sequence to simplify
          */
         [[nodiscard]] virtual OperatorSequence simplify_as_moment(OperatorSequence&& seq) const;


        /**
         * Use context to conjugate operator sequence.
         * @param seq The operator sequence to conjugate.
         */
        [[nodiscard]] virtual OperatorSequence conjugate(const OperatorSequence& seq) const;

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
         [[nodiscard]] size_t hash(const sequence_storage_t& rawSeq) const noexcept {
             return this->hasher(rawSeq);
         }

         /**
          * Get handle to the hasher
          */
          [[nodiscard]] const ShortlexHasher& the_hasher() const noexcept {
              return this->hasher;
          }

         /**
          * Generates a formatted string representation of an operator sequence
          */
          [[nodiscard]] virtual std::string format_sequence(const OperatorSequence& seq) const;

         /**
          * Generates a formatted string representation of an untreated sequence
          */
          [[nodiscard]] virtual std::string format_raw_sequence(const sequence_storage_t& seq) const;

         /**
          * Summarize the context as a string.
          */
         [[nodiscard]] virtual std::string to_string() const;

         /**
          * Gets a generator for operator sequences in this context
          */
         [[nodiscard]] const OperatorSequenceGenerator&
         operator_sequence_generator(size_t level, bool conjugated = false) const;

         /**
          * Gets a generator for operator sequences in this context
          */
         [[nodiscard]] const WordList& osg_list() const noexcept { return *this->word_list; }

         /**
          * Gets a generator for operator sequences in this context
          */
         [[nodiscard]] WordList& osg_list() noexcept { return *this->word_list; }

         /**
          * Gets an operator sequence, but only if it is 'canonical' (i.e. no simplifications performed on it)
          */
         [[nodiscard]] virtual std::optional<OperatorSequence> get_if_canonical(const sequence_storage_t& sequence) const;

    protected:
        /**
         * Instantiate an OSG of the requested length.
         * (May be overloaded to use more efficient enumerations for specific scenarios).
         * @param word_length The maximum length word in the OSG.
         * @return Owning pointer to newly created OSG.
         */
        [[nodiscard]] virtual std::unique_ptr<OperatorSequenceGenerator> new_osg(size_t word_length) const;


    public:
         friend std::ostream& operator<< (std::ostream& os, const Context& context);

         friend class WordList;

    };

}