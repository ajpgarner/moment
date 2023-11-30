/**
 * pauli_context.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_context.h"

#include "nearest_neighbour_index.h"
#include "pauli_dictionary.h"
#include "pauli_osg.h"

#include "utilities/shift_sorter.h"

#include "../contextual_os.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace Moment::Pauli {

    namespace {

        constexpr static oper_name_t cayley_table_ixyz[16] = {0,  1,  2,  3,
                                                              1,  0,  3, -2,
                                                              2, -3,  0,  1,
                                                              3,  2, -1,  0};

        /**
         * Multiply two Pauli matrices
         * @param left Left pauli matrix: 0 = X, 1 = Y, 2 = Z
         * @param right Right pauli matrix: 0 = X, 1 = Y, 2 = Z
         * @return Product matrix, 0 = I, 1 = iX, 2 = iY, 3 = iZ, -1 = -iX, -2 = -iY, -3 = -iZ
         */
        [[nodiscard]] inline constexpr oper_name_t
        multiply_pauli(const oper_name_t left, const oper_name_t right) {
            assert(left >= 0 && left < 3);
            assert(right >= 0 && right < 3);
            return cayley_table_ixyz[((left+1)<<2) + (right+1)];
        }

        /**
          * Multiply two Pauli matrices, treating 0 as ID.
          * NB: For products with 0, no imaginary elements.
          * @param left Left pauli matrix: 0 = I, 1 = X, 2 = Y, 3 = Z
          * @param right Right pauli matrix: 0 = I, 1 = X, 2 = Y, 3 = Z
          * @return Product matrix
          *
          */
        [[nodiscard]] inline constexpr oper_name_t
        multiply_pauli_with_id(const oper_name_t left, const oper_name_t right) {
            assert(left >= 0 && left < 4);
            assert(right >= 0 && right < 4);
            return cayley_table_ixyz[(left<<2) + right];
        }

        /**
         * Orient two qubits in a chain to canonical position
         */
        [[nodiscard]] OperatorSequence simplify_pair_as_moment(const PauliContext& context,
                                                               const oper_name_t first, const oper_name_t second,
                                                               SequenceSignType the_sign) {
            assert(context.qubit_size >= 2);
            const auto first_qubit = static_cast<oper_name_t>(first / 3);
            const auto first_pauli = static_cast<oper_name_t>(first % 3);
            const auto second_qubit = static_cast<oper_name_t>(second / 3);
            const auto second_pauli = static_cast<oper_name_t>(second % 3);
            assert((second_qubit - first_qubit) > 0);

            sequence_storage_t output_data;
            output_data.reserve(2);

            // TODO: Case vs. perfect aliasing; then tie-break with X < Y < Z

            // Is right-ward gap from 1st to 2nd qubit smaller than gap from 2nd to 1st?
            if ((2*second_qubit) < (context.qubit_size + (2* first_qubit))) {
                // Then, make 1st qubit at index 0, and 2nd at distance between them
                output_data.emplace_back(first_pauli);
                output_data.emplace_back((second_qubit-first_qubit)*3 + second_pauli);
            } else {
                // Otherwise, make 2nd qubit at index 0, and 2nd at the first on the wrap-distance
                assert(context.qubit_size > (second_qubit - first_qubit)); // second could be at N-1, and first at 0
                output_data.emplace_back(second_pauli);
                output_data.emplace_back((context.qubit_size - second_qubit + first_qubit)*3 + first_pauli);
            }
            const auto hash = context.hash(output_data);
            return OperatorSequence{OperatorSequence::ConstructRawFlag{}, std::move(output_data), hash, context,
                                    the_sign};
        }

        /**
         * Orient three or more qubits in a chain to canonical position
         */
        [[nodiscard]] OperatorSequence
        simplify_three_or_more_as_moment(const PauliContext& context, OperatorSequence&& seq) {
            // Prepare distances into (thread-local) scratch pad:
            const size_t word_size = seq.size();
            thread_local SmallVector<oper_name_t, 32> qubits;
            assert(word_size > 2);
            assert(word_size <= 32);
            for (size_t q = 0; q < (word_size - 1); ++q) {
                qubits[q] = static_cast<oper_name_t>((seq[q+1] / 3) - (seq[q] / 3));
            }
            qubits[word_size-1] = static_cast<oper_name_t>(context.qubit_size + (seq[0] / 3) - (seq[word_size-1] / 3));

            // Find which qubit we should take as the first
            ShiftSorter<oper_name_t> sorter;
            const size_t leading_qubit_index = sorter(std::span<const oper_name_t>{qubits.get(), word_size});

            // Prepare output
            sequence_storage_t output_data;
            output_data.reserve(word_size);

            // Easy case: no-cycling required
            if (0 == leading_qubit_index) {
                // Looks redundant, contains an implicit floor operation:
                const auto operator_offset = static_cast<oper_name_t>((seq[0] / 3) * 3);

                for (auto op: seq) {
                    output_data.emplace_back(op - operator_offset);
                }
            } else {
                // Start in the middle
                const auto leading_qubit = static_cast<oper_name_t >(seq[leading_qubit_index] / 3);
                const auto end_operator_offset = static_cast<oper_name_t>(leading_qubit * 3);
                for (size_t i = leading_qubit_index; i < word_size; ++i) {
                    output_data.emplace_back(seq[i] - end_operator_offset);
                } // This makes the qubit at the leading qubit index as if it was at qubit 0.

                // Now, we must offset the qubits at the front
                const auto begin_operator_offset = static_cast<oper_name_t>((context.qubit_size - leading_qubit) * 3);
                for (size_t i = 0; i < leading_qubit_index; ++i) {
                    output_data.emplace_back(seq[i] + begin_operator_offset);
                }
            }

            const auto hash = context.hash(output_data);
            return OperatorSequence{OperatorSequence::ConstructRawFlag{}, std::move(output_data),
                                    hash, context, seq.get_sign()};
        }

        /** Orient two qubits in 2D space */
        [[nodiscard]] OperatorSequence simplify_pair_as_moment_2D(const PauliContext& context,
                                                                  const oper_name_t first_op,
                                                                  const oper_name_t second_op,
                                                                  SequenceSignType the_sign) {
            const oper_name_t first_qubit = first_op / 3;
            const oper_name_t second_qubit = second_op / 3;
            throw errors::bad_pauli_context{"Don't know how to simplify pair as moment in lattice."};
        }

        [[nodiscard]] OperatorSequence
        simplify_three_or_more_as_moment_2D(const PauliContext& context, OperatorSequence&& seq) {
            throw errors::bad_pauli_context{"Don't know how to simplify word as moment in lattice."};
        }
    }

    PauliContext::PauliContext(const oper_name_t qubits, const bool is_wrapped, const bool tx_sym,
                               const oper_name_t col_height_in)
        : Context{static_cast<size_t>(qubits*3)}, qubit_size{qubits}, wrap{is_wrapped}, translational_symmetry{tx_sym},
            col_height{col_height_in},
            row_width{(col_height_in> 0) ? static_cast<oper_name_t>(qubits / col_height_in)
                                         : static_cast<oper_name_t>(0)} {
        // Check validity of col_height
        if (col_height != 0) {
            if (col_height < 0) {
                throw errors::bad_pauli_context{"Row width must be a positive integer or zero."};
            }
            auto remainder = qubits % col_height;
            if (remainder != 0) {
                throw errors::bad_pauli_context{"Row width must be a divisor of the number of qubits."};
            }
            assert((row_width * col_height) == qubit_size);
        }
        if (translational_symmetry) {
            if (!wrap) {
                throw errors::bad_pauli_context{"Translational symmetry cannot be imposed on non-wrapping scenarios"};
            }
        }


        // Replace with a dictionary that can handle nearest-neighbour NPA sublevels.
        this->replace_dictionary(std::make_unique<PauliDictionary>(*this));
        this->dictionary_ptr = dynamic_cast<PauliDictionary*>(&this->dictionary());
        assert(this->dictionary_ptr != nullptr);
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
        oper_name_t last_pauli = 1+(*read_iter % 3);

        ++read_iter;

        while (read_iter != read_iter_end) {
                const oper_name_t current_op = *read_iter;
                const oper_name_t current_party = current_op / 3;
                const oper_name_t current_pauli = 1+(current_op % 3);

                // If onto a new party, advance and simply copy
                if (current_party != last_party) {

                    // Multiplication resulted in non-trivial Pauli operator
                    if (last_pauli != 0) {
                        // Otherwise, non-trivial multiplication, so write
                        (*write_iter) = (last_party * 3) + last_pauli - 1;
                        ++write_iter;
                        assert(write_iter != read_iter_end); // Write should always trail read.
                    }


                    last_party = current_party;
                    last_pauli = current_pauli;
                    ++read_iter;
                    continue;
                }

                if (last_pauli != 0) {
                    last_pauli = multiply_pauli_with_id(last_pauli, current_pauli);
                    if (last_pauli > 0) {
                        sign = sign * SequenceSignType::Imaginary;
                    } else if (last_pauli < 0) {
                        sign = sign * SequenceSignType::NegativeImaginary;
                        last_pauli = -last_pauli;
                    }
                } else {
                    last_pauli = current_pauli;
                }

                // Advance read iterator
                ++read_iter;
        }

        // Write last op
        if (last_pauli != 0) {
            (*write_iter) = (last_party * 3) + last_pauli - 1;
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
                auto pauli_product = multiply_pauli(lhs_pauli, rhs_pauli);
                if (pauli_product > 0) {
                    result.emplace_back(3*lhs_qubit + (pauli_product-1));
                    sign = sign * SequenceSignType::Imaginary;
                }  else if (pauli_product < 0) {
                    result.emplace_back(3*lhs_qubit + (-pauli_product-1));
                    sign = sign * SequenceSignType::NegativeImaginary;
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

    OperatorSequence PauliContext::commutator(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
        const auto prefactor_sign = lhs.get_sign() * rhs.get_sign();
        auto result = lhs * rhs;
        if (is_imaginary(prefactor_sign) == result.imaginary()) {
            result.set_to_zero();
        }
        return result;
    }

    OperatorSequence PauliContext::anticommutator(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
        const auto prefactor_sign = lhs.get_sign() * rhs.get_sign();
        auto result = lhs * rhs;
        if (is_imaginary(prefactor_sign) != result.imaginary()) {
            result.set_to_zero();
        }
        return result;
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
        if (seq.zero()) {
            os.os << "0";
            return;
        }

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


        if (seq.empty()) {
            contextual_os.os << "I";
        } else {
            for (const auto &oper: seq) {
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
                contextual_os.os << (qubit + 1); // MATLAB indexing...
            }
        }
        if (contextual_os.format_info.show_braces) {
            contextual_os.os << ">";
        }
    }

    std::string PauliContext::to_string() const {
        std::stringstream ss;
        ss << "Pauli context over "
           << this->qubit_size << " " << ((this->qubit_size !=1 ? "qubits" : "qubit"))
           << " (" << this->operator_count << " operators)";
        if (this->is_lattice()) {
            ss << " in " << this->row_width << " x " << this->col_height << " lattice";
        }

        if (this->wrap) {
            ss << " with wrapping";
            if (this->translational_symmetry) {
                ss << " and translational symmetry";
            }
        }

        ss << ".\n";

        return ss.str();
    }

    const Moment::OperatorSequenceGenerator&
    PauliContext::operator_sequence_generator(const NearestNeighbourIndex& index, bool conjugate) const {
        // NB: Ignore conjugate because everything is Hermitian in this scenario
        const auto& dictionary = dynamic_cast<const PauliDictionary&>(this->dictionary());
        const auto& osg_pair = dictionary.NearestNeighbour(index);
        return osg_pair();
    }

    std::unique_ptr<OperatorSequenceGenerator> PauliContext::new_osg(const size_t word_length) const {
        return std::make_unique<PauliSequenceGenerator>(*this, word_length);
    }

    OperatorSequence PauliContext::simplify_as_moment(OperatorSequence&& seq) const {
        // If no symmetry, just return
        if (!this->translational_symmetry) [[unlikely]] {
            return seq;
        }
        assert(this->wrap);

        // Special case: empty sequences
        if (seq.empty()) {
            return seq;
        }

        // Otherwise action depends on size
        const auto word_size = seq.size();

        // Special case: single operator
        if (1 == word_size) {
            const auto pauli_op = static_cast<oper_name_t>(*seq.begin() % 3);
            return OperatorSequence{OperatorSequence::ConstructRawFlag{}, sequence_storage_t{pauli_op},
                                    static_cast<uint64_t>(pauli_op+2), *this, seq.get_sign()};
        }

        // Remaining cases will depend on if lattice 2D or not:
        if (this->is_lattice()) {
            if (2 == word_size) {
                return simplify_pair_as_moment_2D(*this, seq[0], seq[1], seq.get_sign());
            }
            return simplify_three_or_more_as_moment_2D(*this, std::move(seq));
        }


        // Special case: a pair of qubits
        if (2 == word_size) {
          return simplify_pair_as_moment(*this, seq[0], seq[1], seq.get_sign());
        }

        // General case:
        return simplify_three_or_more_as_moment(*this, std::move(seq));
    }

    bool PauliContext::can_be_simplified_as_moment(const OperatorSequence& seq) const {
        assert(this->translational_symmetry);

        // Obvious tests
        if (seq.empty()) {
            return false;
        }

        // If first qubit isn't 0, then definitely can be moved:
        if (*seq.begin() >= 3) {
            return true;
        }

        // TODO: Quicker test for remaining cases


        return Context::can_be_simplified_as_moment(seq);
    }
}