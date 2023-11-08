/**
 * pauli_context.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_context.h"

#include "../contextual_os.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace Moment::Pauli {

    PauliContext::PauliContext(const oper_name_t qubits, const oper_name_t range) noexcept
        : Context{static_cast<size_t>(qubits*3)}, qubit_size{qubits}, moment_matrix_range{range} {

    }

    bool PauliContext::additional_simplification(sequence_storage_t& op_sequence, SequenceSignType &sign) const {
        // Early exit on empty operator sequence
        if (op_sequence.empty()) {
            return false;
        }

        // First, order operators by party
        std::stable_sort(op_sequence.begin(), op_sequence.end(), [](const oper_name_t lhs, const oper_name_t rhs) {
            return (lhs / 3) < (rhs / 3);
        });

        // Note: Pauli simplification can only reduce sequence length

        auto write_iter = op_sequence.begin();
        auto read_iter = write_iter;
        const auto read_iter_end = op_sequence.end();
        assert(read_iter != read_iter_end); // Already returned early if empty string.

        oper_name_t last_party = (*read_iter / 3);
        oper_name_t last_pauli = (*read_iter % 3);

        ++read_iter;

        while (read_iter != read_iter_end) {
                const oper_name_t current_op = *read_iter;
                const oper_name_t current_party = current_op / 3;
                const oper_name_t current_pauli = current_op % 3;

                // If onto a new party, advance and simply copy
                if (current_party != last_party) {

                    // Multiplication resulted in non-trivial Pauli operator
                    if (last_pauli != 3) {
                        // Otherwise, non-trivial multiplication, so write
                        (*write_iter) = (last_party * 3) + last_pauli;
                        ++write_iter;
                        assert(write_iter != read_iter_end); // Write should always trail read.
                    }


                    last_party = current_party;
                    last_pauli = current_pauli;
                    ++read_iter;
                    continue;
                }

                // Multiplication
                switch (last_pauli) {
                    case 0:
                    switch (current_pauli) {
                        case 0: // X X = 1
                            last_pauli = 3;
                            break;
                        case 1: // X Y = i Z
                            last_pauli = 2;
                            sign = sign * SequenceSignType::Imaginary;
                            break;
                        case 2: // X Z = -i Y
                            last_pauli = 1;
                            sign = sign * SequenceSignType::NegativeImaginary;
                            break;
                    }
                    break;
                    case 1:
                    switch (current_pauli) {
                        case 0: // Y X = -i Z
                            last_pauli = 2;
                            sign = sign * SequenceSignType::NegativeImaginary;
                            break;
                        case 1: // Y Y = 1
                            last_pauli = 3;
                            break;
                        case 2: // Y Z = i X
                            last_pauli = 0;
                            sign = sign * SequenceSignType::Imaginary;
                            break;
                    }
                    break;
                    case 2:
                    switch (current_pauli) {
                        case 0: // Z X = iY
                            last_pauli = 1;
                            sign = sign * SequenceSignType::Imaginary;
                            break;
                        case 1: // Z Y = -i X
                            last_pauli = 0;
                            sign = sign * SequenceSignType::NegativeImaginary;
                            break;
                        case 2: // Z Z = 1
                            last_pauli = 3;
                            break;
                    }
                    break;
                    case 3: // ID x [x/y/z]
                        last_pauli = current_pauli;
                        break;
                }

                // Advance read iterator
                ++read_iter;
        }

        // Write last op
        if (last_pauli != 3) {
            (*write_iter) = (last_party * 3) + last_pauli;
            ++write_iter;
        }

        // Clear rest of sequence
        if (write_iter != read_iter_end) {
            op_sequence.erase(write_iter, read_iter_end);
        }

        // Pauli simplification never resolves to 0
        return false;
    }

    void PauliContext::multiply(OperatorSequence &lhs, const OperatorSequence &rhs) const {

        // Get initial sign of product
        SequenceSignType sign = lhs.get_sign() * rhs.get_sign();

        // First, if RHS is empty...
        if (rhs.empty()) {
            // If RHS is zero, product is zero
            if (rhs.zero()) {
                lhs.raw().clear();
                lhs.set_sign(SequenceSignType::Positive);
                lhs.rehash(0);
                return;
            } else {
                // RHS is identity, -1, i or -i, so only do sign change.
                lhs.set_sign(sign);
                return;
            }
        }

        // Second, if LHS is empty and not zero, copy in RHS
        if (lhs.empty()) {
            if (lhs.zero()) {
                // LHS is zero, so nothing changes.
                return;
            } else {
                assert(!rhs.empty()); // <- would have already returned.
                lhs.raw().insert(lhs.raw().begin(), rhs.begin(), rhs.end());
                lhs.set_sign(sign);
                lhs.rehash(rhs.hash());
                return;
            }
        }

        // Both sides are non-trivial.
        sequence_storage_t result;

        const auto * lhs_iter = lhs.raw().begin();
        const auto * const lhs_iter_end = lhs.raw().end();
        const auto * rhs_iter = rhs.raw().begin();
        const auto * const rhs_iter_end = rhs.raw().end();

        oper_name_t lhs_op = *lhs_iter;
        oper_name_t lhs_qubit = lhs_op / static_cast<oper_name_t>(3);
        oper_name_t lhs_pauli = lhs_op % static_cast<oper_name_t>(3);

        oper_name_t rhs_op = *rhs_iter;
        oper_name_t rhs_qubit = rhs_op / static_cast<oper_name_t>(3);
        oper_name_t rhs_pauli = rhs_op % static_cast<oper_name_t>(3);

        while (true) {
            if (lhs_qubit < rhs_qubit) {
                // Copy LHS to output
                result.emplace_back(lhs_op);
                ++lhs_iter;

                // No more LHS...
                if (lhs_iter == lhs_iter_end) {
                    // Copy rest of RHS
                    result.insert(result.end(), rhs_iter, rhs_iter_end);
                    break;
                } else {
                    // Get next LHS op
                    lhs_op = *lhs_iter;
                    lhs_qubit = lhs_op / static_cast<oper_name_t>(3);
                    lhs_pauli = lhs_op % static_cast<oper_name_t>(3);
                }

                // Advance LHS
            } else if (lhs_qubit > rhs_qubit) {
                // Copy RHS to output
                result.emplace_back(rhs_op);
                ++rhs_iter;

                // No more RHS...
                if (rhs_iter == rhs_iter_end) {
                    result.insert(result.end(), lhs_iter, lhs_iter_end);
                    break;
                } else {
                    // Get next RHS op
                    rhs_op = *rhs_iter;
                    rhs_qubit = rhs_op / static_cast<oper_name_t>(3);
                    rhs_pauli = rhs_op % static_cast<oper_name_t>(3);
                }

                // Advance RHS
            } else {
                assert(lhs_qubit == rhs_qubit);
                // XX = YY = ZZ = 1, so skip....
                switch (lhs_pauli) {
                    case 0: // X
                    switch (rhs_pauli) {
                        case 0: // X
                            // X X = 1
                            break;
                        case 1: // Y
                            result.emplace_back(3*lhs_qubit + 2); // X Y = iZ
                            sign = sign * SequenceSignType::Imaginary;
                            break;
                        case 2: // Z
                            result.emplace_back(3*lhs_qubit + 1); // X Z = -iY;
                            sign = sign * SequenceSignType::NegativeImaginary;
                            break;
                        default:
                            assert(false);
                    }
                    break;
                    case 1: // Y
                        switch (rhs_pauli) {
                            case 0: // X
                                result.emplace_back(3*lhs_qubit + 2); // Y X = -iZ
                                sign = sign * SequenceSignType::NegativeImaginary;
                                break;
                            case 1: // Y
                                // Y Y = 1
                                break;
                            case 2: // Z
                                result.emplace_back(3*lhs_qubit); // Y Z = iX;
                                sign = sign * SequenceSignType::Imaginary;
                                break;
                            default:
                                assert(false);
                        }
                    break;
                    case 2: // Z
                        switch (rhs_pauli) {
                            case 0: // X
                                result.emplace_back(3*lhs_qubit + 1); // Z X = iY;
                                sign = sign * SequenceSignType::Imaginary;
                                break;
                            case 1: // Y
                                result.emplace_back(3*lhs_qubit); // Z Y = -iX
                                sign = sign * SequenceSignType::NegativeImaginary;
                                break;
                            case 2: // Z
                                // Z Z = 1;
                                break;
                            default:
                                assert(false);
                        }
                    break;
                    default:
                        assert(false);
                }

                // Advance LHS & RHS
                ++lhs_iter;
                ++rhs_iter;


                // Is LHS done...?
                if (lhs_iter == lhs_iter_end) {
                    // Copy rest of RHS
                    result.insert(result.end(), rhs_iter, rhs_iter_end);
                    break;
                } else {
                    // Get next LHS op
                    lhs_op = *lhs_iter;
                    lhs_qubit = lhs_op / static_cast<oper_name_t>(3);
                    lhs_pauli = lhs_op % static_cast<oper_name_t>(3);
                }

                // Is RHS done...?
                if (rhs_iter == rhs_iter_end) {
                    // Copy rest of LHS
                    result.insert(result.end(), lhs_iter, lhs_iter_end);
                    break;
                } else {
                    // Get next RHS op
                    rhs_op = *rhs_iter;
                    rhs_qubit = rhs_op / static_cast<oper_name_t>(3);
                    rhs_pauli = rhs_op % static_cast<oper_name_t>(3);
                }

            }
        }

        // Move in multiplied sequence
        lhs.raw() = std::move(result);
        lhs.set_sign(sign);
        lhs.rehash(this->hasher);
    }

    OperatorSequence PauliContext::conjugate(const OperatorSequence &seq) const {
        return OperatorSequence{OperatorSequence::ConstructRawFlag{},
                                seq.raw(), seq.hash(), *this, Moment::conjugate(seq.get_sign())};
    }

    OperatorSequence PauliContext::sigmaX(const oper_name_t qubit, SequenceSignType sign) const {
        assert(qubit < this->qubit_size);
        const oper_name_t op_number = 3*qubit;
        return OperatorSequence{OperatorSequence::ConstructRawFlag{},
                                sequence_storage_t{op_number}, static_cast<uint64_t>(op_number+2), *this, sign};
    }

    OperatorSequence PauliContext::sigmaY(const oper_name_t qubit, SequenceSignType sign) const {
        assert(qubit < this->qubit_size);
        const oper_name_t op_number = (3*qubit) + 1;
        return OperatorSequence{OperatorSequence::ConstructRawFlag{},
                                sequence_storage_t{op_number}, static_cast<uint64_t>(op_number+2), *this, sign};
    }

    OperatorSequence PauliContext::sigmaZ(const oper_name_t qubit, SequenceSignType sign) const {
        assert(qubit < this->qubit_size);
        const oper_name_t op_number = (3*qubit) + 2;
        return OperatorSequence{OperatorSequence::ConstructRawFlag{},
                                sequence_storage_t{op_number}, static_cast<uint64_t>(op_number+2), *this, sign};
    }

    void PauliContext::format_sequence(ContextualOS &os, const OperatorSequence &seq) const {
        switch (seq.get_sign()) {
            case SequenceSignType::Positive:
                break;
            case SequenceSignType::Imaginary:
                os.os << "i";
                break;
            case SequenceSignType::Negative:
                os.os << "-";
                break;
            case SequenceSignType::NegativeImaginary:
                os.os << "-i";
                break;
        }
        this->format_raw_sequence(os, seq.raw());
    }

    void PauliContext::format_raw_sequence(ContextualOS& contextual_os, const sequence_storage_t &seq) const {

        if (contextual_os.format_info.show_braces) {
            contextual_os.os << "<";
        }

        for (const auto& oper : seq) {
            oper_name_t qubit = oper / 3;
            oper_name_t pauli_op = oper % 3;
            switch (pauli_op) {
                case 0:
                    contextual_os.os << "X";
                    break;
                case 1:
                    contextual_os.os << "Y";
                    break;
                case 2:
                    contextual_os.os << "Z";
                    break;
                default:
                    assert(false);
            }
            contextual_os.os << (qubit+1); // MATLAB indexing...
        }
        if (contextual_os.format_info.show_braces) {
            contextual_os.os << ">";
        }
    }

    std::string PauliContext::to_string() const {
        std::stringstream ss;
        ss << "Pauli context over "
           << this->qubit_size << " " << ((this->qubit_size !=1 ? "qubits" : "qubit"))
           << " (" << this->operator_count << " operators).\n";

        return ss.str();
    }
}