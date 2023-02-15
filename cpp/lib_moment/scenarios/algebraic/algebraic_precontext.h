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
        const oper_name_t num_operators;
        const bool self_adjoint;
        const ShortlexHasher hasher;

    private:
        const oper_name_t conj_offset;

    public:
        explicit AlgebraicPrecontext(const oper_name_t op_count, const bool hermitian = true)
            : num_operators{hermitian ? op_count : static_cast<oper_name_t>(2 * op_count)},
              self_adjoint{hermitian},
               hasher{static_cast<size_t>(num_operators)},
              conj_offset{op_count} // not the 2 * value!
           {

        }

        AlgebraicPrecontext(const AlgebraicPrecontext& rhs) = default;


        [[nodiscard]] sequence_storage_t conjugate(const sequence_storage_t& rawSeq) const;

        [[nodiscard]] HashedSequence conjugate(const HashedSequence& seq) const;

        [[nodiscard]] inline size_t hash(const sequence_storage_t& rawSeq) const {
            return this->hasher(rawSeq);
        }
    };
};