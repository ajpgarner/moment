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
            hermitian{false}, antihermitian{false}, real_index{-1}, img_index{-1} {

        int compare = OperatorSequence::compare_same_negation(*opSeq, *conjSeq);
        if (1 == compare) {
            this->hermitian = true;
        } else if (-1 == compare) {
            this->antihermitian = true;
        }
    }

    std::ostream& operator<<(std::ostream& os, const Symbol& seq) {
        const bool has_fwd = seq.opSeq.has_value();

        os << "#" << seq.id << ":\t";

        if (has_fwd) {
            std::string fwd_sequence = seq.formatted_sequence();
            os << fwd_sequence;
            if (fwd_sequence.length() >= 80) {
                os << "\n\t";
            } else {
                os << ":\t";
            }
        } else {
            os << "<No sequence>:\t";
        }

        if (seq.real_index>=0) {
            if (seq.img_index>=0) {
                os << "Complex";
            } else {
                os << "Real";
            }
        } else if (seq.img_index>=0) {
            os << "Imaginary";
        } else {
            os << "Zero";
        }

        if (seq.hermitian) {
            os << ", Hermitian";
        }
        if (seq.real_index>=0) {
            os << ", Re#=" << seq.real_index;
        }
        if (seq.img_index>=0) {
            os << ", Im#=" << seq.img_index;
        }

        if (has_fwd) {
            os << ", hash=" << seq.hash();
            if (seq.hash_conj() != seq.hash()) {
                os << "/" << seq.hash_conj();
            }
        } else {
            os << ", unhashable";
        }
        return os;
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