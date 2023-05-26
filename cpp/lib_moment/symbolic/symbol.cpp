/**
 * symbol.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol.h"

#include <sstream>

namespace Moment {
    Symbol::Symbol(OperatorSequence sequence,
                   OperatorSequence conjSequence):
            opSeq{std::move(sequence)},
            conjSeq{std::move(conjSequence)},
            hermitian{false}, antihermitian{false}, real_index{-1}, img_index{-1},
            fwd_sequence_str{opSeq->formatted_string()} {

        int compare = OperatorSequence::compare_same_negation(*opSeq, *conjSeq);
        if (1 == compare) {
            this->hermitian = true;
        } else if (-1 == compare) {
            this->antihermitian = true;
        }
    }


    std::string Symbol::formatted_sequence() const {
        if (this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "#" << this->id;
        return ss.str();
    }

    std::string Symbol::formatted_sequence_conj() const {
        if (this->conjSeq.has_value()) {
            return this->conjSeq->formatted_string();
        } else if (this->hermitian && this->opSeq.has_value()) {
            return this->opSeq->formatted_string();
        }

        std::stringstream ss;
        ss << "#" << this->id;
        if (!this->hermitian) {
            ss << "*";
        }
        return ss.str();
    }
}