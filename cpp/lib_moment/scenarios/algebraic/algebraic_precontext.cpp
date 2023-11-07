/**
 * algebraic_precontext.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_precontext.h"
#include <algorithm>

namespace Moment::Algebraic {
    sequence_storage_t AlgebraicPrecontext::conjugate(const sequence_storage_t &seq) const {

        // Do nothing on empty sequence
        if (seq.empty()) {
            return sequence_storage_t{};
        }

        // Prepare output
        sequence_storage_t output;
        const size_t raw_size = seq.size();
        output.reserve(raw_size);

        switch (this->conj_mode) {
            case ConjugateMode::SelfAdjoint:
                std::reverse_copy(seq.begin(), seq.end(), std::back_inserter(output));
                break;
            case ConjugateMode::Bunched:
                for (size_t out_index = 0; out_index < raw_size; ++out_index) {
                    const size_t in_index = raw_size - out_index - 1;
                    // A1...An <-> B1...Bn
                    output.emplace_back((seq[in_index] + this->raw_operators) % this->num_operators);
                }
                break;
            case ConjugateMode::Interleaved:
                for (size_t out_index = 0; out_index < raw_size; ++out_index) {
                    const size_t in_index = raw_size - out_index - 1;
                    output.emplace_back(seq[in_index] ^ 0x1); // op 0<->1, 2<->3, etc.
                }
                break;
        }

        return output;
    }

    [[nodiscard]] HashedSequence AlgebraicPrecontext::conjugate(const HashedSequence& seq) const {
        if (seq.zero()) {
            return HashedSequence{true};
        }
        return HashedSequence{this->conjugate(seq.raw()), this->hasher, Moment::conjugate(seq.get_sign())};
    }
}

