/**
 * symbol.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol.h"
#include "scenarios/context.h"

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

    void Symbol::output_uncontextual_info(std::ostream& os) const {
        if (this->real_index>=0) {
            if (this->img_index>=0) {
                os << "Complex";
            } else {
                os << "Real";
            }
        } else if (this->img_index>=0) {
            os << "Imaginary";
        } else {
            os << "Zero";
        }

        if (this->hermitian) {
            os << ", Hermitian";
        }
        if (this->real_index>=0) {
            os << ", Re#=" << this->real_index;
        }
        if (this->img_index>=0) {
            os << ", Im#=" << this->img_index;
        }
        const bool has_fwd = this->opSeq.has_value();
        if (has_fwd) {
            os << ", hash=" << this->hash();
            if (this->hash_conj() != this->hash()) {
                os << "/" << this->hash_conj();
            }
        } else {
            os << ", unhashable";
        }
    }

    std::ostream& operator<<(ContextualOS& os, const Symbol& symbol) {
        os << "#" << symbol.id << ":\t";

        if (symbol.opSeq.has_value()) {
            os.context.format_sequence(os, symbol.opSeq.value());
        } else {
            os.context.format_sequence_from_symbol_id(os, symbol.id, false);
        }
        os << ":\t";

        symbol.output_uncontextual_info(os.os);
        return os;
    }


    std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {

        os << "#" << symbol.id << ":\t";

        if (symbol.opSeq.has_value()) {
            // Uncontextual, fallback.
            std::string fwd_sequence = symbol.formatted_sequence();
            os << fwd_sequence;
            if (fwd_sequence.length() >= 80) {
                os << "\n\t";
            } else {
                os << ":\t";
            }
        } else {
            // Uncontextual, unknowable
            os << "<No sequence>:\t";
        }

        symbol.output_uncontextual_info(os);
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