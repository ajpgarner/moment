/**
 * algebraic_precontext.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "hashed_sequence.h"
#include "utilities/shortlex_hasher.h"

#include <cassert>

namespace Moment::Algebraic {
    class AlgebraicPrecontext {
    public:
        /** The number of operators, including added conjugates. */
        const oper_name_t num_operators;

        /** The number of operators, excluding added conjugates. */
        const oper_name_t raw_operators;

        /** The way operators conjugate, or if they are self-adjoint */
        const enum class ConjugateMode : int8_t {
            /** All operators are their own adjoint */
            SelfAdjoint,
            /** All ops followed by all conjugate ops: A, ... Z, A*... , Z*. */
            Bunched,
            /** Each op followed by its own conjugate: A, A*, ... Z, Z*. */
            Interleaved
        } conj_mode = ConjugateMode::SelfAdjoint;

        /** The short-lex hasher associated with this (pre-)context */
        const ShortlexHasher hasher;

    public:
        explicit AlgebraicPrecontext(const oper_name_t op_count, ConjugateMode mode = ConjugateMode::SelfAdjoint)
            : num_operators{(mode == ConjugateMode::SelfAdjoint) ? op_count : static_cast<oper_name_t>(2 * op_count)},
              raw_operators{op_count}, // not the 2 * value!
              conj_mode{mode},
              hasher{static_cast<size_t>(num_operators)} {

        }

        AlgebraicPrecontext(const AlgebraicPrecontext& rhs) = default;

        [[nodiscard]] inline bool self_adjoint() const noexcept {
            return this->conj_mode == ConjugateMode::SelfAdjoint;
        }

        [[nodiscard]] sequence_storage_t conjugate(const sequence_storage_t& rawSeq) const;

        [[nodiscard]] HashedSequence conjugate(const HashedSequence& seq) const;

        [[nodiscard]] inline size_t hash(const sequence_storage_t& rawSeq) const {
            return this->hasher(rawSeq);
        }
    };
};