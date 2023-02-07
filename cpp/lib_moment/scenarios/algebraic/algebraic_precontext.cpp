/**
 * algebraic_precontext.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "algebraic_precontext.h"
#include <algorithm>

namespace Moment::Algebraic {
    sequence_storage_t AlgebraicPrecontext::conjugate(const sequence_storage_t &seq) const {

        sequence_storage_t output;

        // Reserve
        const size_t raw_size = seq.size();
        output.reserve(raw_size);

        if (this->self_adjoint) {
            // Just flip string
            std::reverse_copy(seq.begin(), seq.end(), std::back_inserter(output));
        } else {
            assert(num_operators > 0);
            // Flip string, and offset operators
            for (size_t out_index = 0; out_index < raw_size; ++out_index) {
                const size_t in_index = raw_size - out_index - 1;
                // A1...An <-> B1...Bn
                output.emplace_back((seq[in_index] + this->conj_offset) % this->num_operators);
            }
        }
        return output;
    }

    [[nodiscard]] HashedSequence AlgebraicPrecontext::conjugate(const HashedSequence& seq) const {
        if (seq.zero()) {
            return HashedSequence{true};
        }
        return HashedSequence{this->conjugate(seq.raw()), this->hasher};
    }
}

