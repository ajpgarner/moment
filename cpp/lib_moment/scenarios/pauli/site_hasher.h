/**
 * site_hasher.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"
#include "hashed_sequence.h"

#include <cassert>

#include <array>
#include <bit>
#include <bitset>
#include <concepts>
#include <limits>
#include <span>
#include <vector>

namespace Moment::Pauli {

    class SiteHasherBase {
    public:
        using storage_t = uint64_t;

        /** The number of qubits we can fit on each slide */
        constexpr static oper_name_t qubits_per_slide = sizeof(storage_t) * 4; // should be 32

        /** Number of qubits in this particular hasher instance. */
        const size_t qubits;

        /** The size the major index (i.e. column size), in lattice mode. */
        const size_t column_height;

        /** The total number of columns (i.e. row size), in lattice mode. */
        const size_t row_width;

        /** Number of qubits on the final slide of this hasher. */
        const size_t qubits_on_final_slide;

        /** The mask for final slide when rotating */
        const storage_t final_slide_mask;

        /** The mask for a single column (max size, 32 qubits). */
        const storage_t column_mask;

        /**
         * Construct a site-hasher
         * @param qubit_count The maximum number of qubits in the hasher.
         * @param col_size The number of qubits in a column.
         */
        explicit constexpr SiteHasherBase(const size_t qubit_count, const size_t col_size = 0)
                : qubits{qubit_count},
                  column_height{col_size > 0 ? col_size : qubit_count},
                  row_width{col_size > 0 ? (qubit_count / col_size) : 1},
                  qubits_on_final_slide{calculate_last_slide_qubit_count(qubit_count)},
                  final_slide_mask{calculate_mask_from_qubits(qubits_on_final_slide)},
                 column_mask(calculate_mask_from_qubits(col_size)) {
            assert(column_height * row_width == qubits);
        }

    private:
        /**
         * Calculates number of qubits on last slide
         */
        [[nodiscard]] constexpr static size_t
        calculate_last_slide_qubit_count(const std::make_signed<size_t>::type qubit_count) noexcept {
            return ((qubit_count - 1)) % (std::numeric_limits<storage_t>::digits/2) + 1;
        }

    protected:
        /**
        * Calculates the a bit mask for N qubits
        * @param num_qubits The number of qubits N
        * @return An integer with the first N bits set
        */
        [[nodiscard]] constexpr static storage_t
        calculate_mask_from_bits(const size_t num_bits) noexcept {
            if ((2 * qubits_per_slide == num_bits) || (0 == num_bits)) {
                return ~static_cast<storage_t>(0); // 0xff...ff
            }

            // e.g (1<<63)=0x80...; -1 sets all but the highest bit.
            return static_cast<storage_t>((static_cast<storage_t>(1) << (num_bits)) - 1);
        }

        /**
         * Calculates the a bit mask for N qubits
         * @param num_qubits The number of qubits N
         * @return An integer with the first N bits set
         */
        [[nodiscard]] constexpr static storage_t
        calculate_mask_from_qubits(const size_t num_qubits) noexcept {
            return calculate_mask_from_bits(num_qubits*2);
        }

    };

    template <size_t num_slides>
    class SiteHasher : public SiteHasherBase {
    public:
        /** Hash result. */
        using Datum = std::array<storage_t, num_slides>;

        /** Maximum number of allowed slides by hasher. */
        constexpr static const size_t slides = num_slides;

        explicit constexpr SiteHasher(const size_t qubit_count, const size_t col_size = 0)
            : SiteHasherBase{qubit_count, col_size} {
            assert(qubit_count <= slides * qubits_per_slide);
        }


        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] constexpr Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
            Datum output;
            output.fill(0);
            for (const auto op: sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                const storage_t slide_num = qubit_number / qubits_per_slide;
                const storage_t slide_offset = qubit_number % qubits_per_slide;
                output[slide_num] += ((pauli_op + 1) << (slide_offset * 2));
            }
            return output;
        }

        [[nodiscard]] inline constexpr Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            return this->hash(sequence);
        }

        /**
         * Reconstruct a sequence from its Pauli site hash.
         */
        [[nodiscard]] sequence_storage_t unhash(const Datum& input) const {

            // In principle reserve() could be called here; but since we almost always have shorter words than the stack
            // limit, in most use cases the time spend counting bits beforehand would result in a pessimization.
            sequence_storage_t output;

            // Cycle through slides, finding non-zero entries
            for (size_t slide = 0; slide < slides; ++slide) {
                storage_t qubit_number = slide * qubits_per_slide;
                storage_t within_slide_cursor = input[slide];
                while (0 != within_slide_cursor) {
                    storage_t qubit_offset = std::countr_zero(within_slide_cursor) / 2;
                    within_slide_cursor >>= (qubit_offset * 2); // Consume bits up to qubit

                    const oper_name_t pauli_op = (within_slide_cursor & 0x3) - 1; // 01 -> X (0), 10 -> Y (1), 11 -> Z (2);

                    qubit_number += qubit_offset;
                    output.emplace_back((qubit_number*3) + pauli_op);

                    // Consume qubit:
                    within_slide_cursor >>= 2;
                    qubit_number += 1;
                }
            }
            return output;
        }


        /**
         * Shift the data about in a chain
         */
        [[nodiscard]] constexpr inline Datum cyclic_shift(const Datum& input, size_t offset) const noexcept {
            // Number of slides that have to be outright pushed around
            offset = offset % this->qubits;

            // If no offset, copy input
            if (0 == offset) {
                return input;
            }

            // Calculate offset parameters
            const size_t front_slide_offset = offset / qubits_per_slide; // Positive offset to slides
            const size_t front_bit_offset = (offset % qubits_per_slide) * 2; // 2 bits per qubit
            const size_t back_offset = (this->qubits - offset);
            const size_t back_slide_offset = back_offset / qubits_per_slide;
            const size_t back_bit_offset = (back_offset % qubits_per_slide) * 2;
            const size_t back_bit_anti_offset = std::numeric_limits<storage_t>::digits - back_bit_offset;

            // Go through input slides and distribution
            Datum output;
            output.fill(0);

            // Right shift start of word to end
            if (0 == front_bit_offset) {
                for (size_t idx = front_slide_offset; idx < slides; ++idx) {
                    output[idx] = input[idx-front_slide_offset];
                }
            } else {
                const size_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;
                output[front_slide_offset] = input[0] << front_bit_offset;
                for (size_t idx = front_slide_offset+1; idx < slides; ++idx) {
                    output[idx] = (input[idx-front_slide_offset] << front_bit_offset)
                                  + (input[idx-front_slide_offset - 1] >> front_bit_anti_offset);
                }
            }

            // Left shift end of word to beginning of output
            if (0 == back_bit_offset) {
                for (ptrdiff_t idx = 0; idx < slides - back_slide_offset; ++idx) {
                    output[idx] |= input[idx + back_slide_offset];
                }
            } else {
                // otherwise 0 offset?
                for (ptrdiff_t idx = 0; idx < slides - back_slide_offset - 1; ++idx) {
                    output[idx] |= (input[idx + back_slide_offset] >> back_bit_offset)
                                   | (input[idx + back_slide_offset+1] << back_bit_anti_offset);
                }
                output[slides - back_slide_offset - 1] |= (input[slides - 1] >> back_bit_offset);
            }

            // Apply mask to final slide
            output[slides-1] &= this->final_slide_mask;

            return output;

        }

        /**
          * Offset along major axis:
          */
        [[nodiscard]] inline constexpr Datum col_shift(const Datum input, const size_t offset) const noexcept {
            return cyclic_shift(input, (offset % this->row_width) * this->column_height);
        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] constexpr Datum row_shift(const Datum& input, size_t offset) const noexcept {

            // Normalize rotation, and get within-column offsets
            offset = offset % this->row_width;
            const storage_t bit_offset = 2 * offset;
            const storage_t bit_anti_offset = (this->column_height * 2) - bit_offset;

            // Skip trivial offset
            if (0 == offset) {
                return input;
            }

            Datum output;
            output.fill(0);

            for (size_t column = 0; column < this->row_width; ++column) {
                const size_t first_slide = (column * this->column_height) / qubits_per_slide;
                const ptrdiff_t offset_slide_one = static_cast<ptrdiff_t>(2)
                                                   * ((column * this->column_height) % qubits_per_slide);
                const ptrdiff_t offset_slide_two = static_cast<ptrdiff_t>(2)
                                                   * (((column + 1) * this->column_height) % qubits_per_slide);

                if ((offset_slide_two <= offset_slide_one) && (offset_slide_two != 0)) {
                    // Column straddles slide boundaries
                    size_t remainder = column_height - (offset_slide_two / 2);
                    const uint64_t remainder_mask = calculate_mask_from_bits(offset_slide_two);
                    storage_t word = ((input[first_slide] >> offset_slide_one) & this->column_mask)
                                        | ((input[first_slide+1] & remainder_mask) << (remainder*2)); // Read word
                    word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask; // Rotate word

                    output[first_slide]   |= (word << offset_slide_one);  // Add word part to first slide
                    output[first_slide+1]  = (word >> (remainder*2));  // Overwrite second slide with  second word part

                } else { // Entirely within first slide
                    // Within slide shift
                    storage_t word = (input[first_slide] >> offset_slide_one) & this->column_mask; // Read word
                    word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask; // Rotate word
                    output[first_slide] |= (word << offset_slide_one); // Write word to output
                }
            }
          return output;
        }

        /**
         * Slice out value of a single column
         */
        [[nodiscard]] constexpr storage_t extract_column(const Datum& input, size_t column) const noexcept {
            assert(column < this->row_width);

            const size_t first_slide = (column * this->column_height) / qubits_per_slide;
            const ptrdiff_t offset_slide_one = static_cast<ptrdiff_t>(2)
                    * ((column * this->column_height) % qubits_per_slide);
            const ptrdiff_t offset_slide_two = static_cast<ptrdiff_t>(2)
                    * (((column + 1) * this->column_height) % qubits_per_slide);

            // First, get bits from first slide
            storage_t output = (input[first_slide] >> offset_slide_one) & this->column_mask;

            // Check if subset spills over to next slide. Works because column_height <= qubits_per_slide.
            if ((offset_slide_two <= offset_slide_one) && (offset_slide_two != 0)) {
                size_t remainder = column_height - (offset_slide_two / 2);
                const uint64_t remainder_mask = calculate_mask_from_bits(offset_slide_two);
                output |= ((input[first_slide+1] & remainder_mask) << (remainder*2));
            }

            return output;
        }

    };

    /**
     * Specialized hasher for up to 32 qubits, stored in one 64-bit number
     */
    template<>
    class SiteHasher<1> : public SiteHasherBase {
    public:
        /** Hash result type. */
        using Datum = storage_t;

        /** Number of slides. */
        constexpr static size_t slides = 1;

        /**
         * Construct a Pauli site hasher for up to 32 qubits
         * @param qubit_count
         * @param col_size The number of qubits per column for a lattice, or set to 0 for a chain.
         */
        explicit constexpr SiteHasher(const size_t qubit_count, const size_t col_size = 0)
                : SiteHasherBase{qubit_count, col_size} {
            assert(qubit_count <= qubits_per_slide);
        }

        /**
         * Hash: Specialization to 1 slide (up to 32 qubits).
         */
        [[nodiscard]] constexpr Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
            // Prepare all zero output
            Datum output{0};

            for (const auto op : sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                output += ((pauli_op+1) << (qubit_number*2));
            }
            return output;
        }

        [[nodiscard]] inline constexpr Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            return this->hash(sequence);
        }


        /**
       * Reconstruct a sequence from its Pauli site hash.
       */
        [[nodiscard]] sequence_storage_t unhash(Datum within_slide_cursor) const {

            // In principle reserve() could be called here; but since we almost always have shorter words than the stack
            // limit, in most use cases the time spend counting bits beforehand would result in a pessimization.
            sequence_storage_t output;

            // Cycle through non-zero entries
            storage_t qubit_number = 0;
            while (0 != within_slide_cursor) {
                storage_t qubit_offset = std::countr_zero(within_slide_cursor) / 2;
                within_slide_cursor >>= (qubit_offset * 2); // Consume bits up to qubit

                const oper_name_t pauli_op = (within_slide_cursor & 0x3) - 1; // 01 -> X (0), 10 -> Y (1), 11 -> Z (2);

                qubit_number += qubit_offset;
                output.emplace_back((qubit_number*3) + pauli_op);

                // Consume qubit:
                within_slide_cursor >>= 2;
                qubit_number += 1;
            }
            return output;
        }


        /**
         * Bit rotation for chain
         */
        [[nodiscard]] constexpr Datum cyclic_shift(const Datum input,  size_t offset) const noexcept {
            offset = 2*(offset % qubits_per_slide);
            if (0 == offset) {
                return input;
            }
            return Datum{((input << offset) & this->final_slide_mask) | (input >> (2 * this->qubits - offset))};
        }

        /**
         * Offset along major axis:
         */
        [[nodiscard]] inline constexpr Datum col_shift(const Datum input, const size_t offset) const noexcept {
            return cyclic_shift(input, (offset % this->row_width) * this->column_height);
        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] constexpr Datum row_shift(const Datum input, size_t offset) const noexcept {
            assert(this->column_height != 0);
            offset = offset % this->column_height;
            if (0 == offset) {
                return input;
            }

            const storage_t bit_offset = 2 * offset;
            const storage_t bit_anti_offset = (this->column_height * 2) - bit_offset;

            Datum output{0};
            for (size_t cIdx = 0; cIdx < this->row_width; ++cIdx) { // Cycle through columns
                storage_t word = this->extract_column(input, cIdx);
                word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask;
                output |= word << (cIdx * this->column_height*2);
            }
            return output;
        }

        /**
         * Slice out hash page from a single column.
         */
        [[nodiscard]] inline constexpr storage_t extract_column(const Datum input, size_t column) const noexcept {
            assert(column < this->row_width);
            return ((input >> (column*this->column_height*2)) & this->column_mask);
        }

    };

    /**
     * Specialized hasher for up to 64 qubits, stored in two 64-bit numbers.
     */
    template<>
    class SiteHasher<2> : public SiteHasherBase {
    public:
        /** Hash result type. */
        using Datum = std::array<storage_t, 2>;

        /** Number of slides. */
        constexpr static size_t slides = 2;

        /** Helper information when splicing a column out from over the slide boundary. */
        const struct BoundaryCalculator {
            /** The mask to apply to the LHS of the boundary */
            uint64_t lhs_mask;

            /** The left shift to apply to the bits on the LHS of the boundary */
            uint64_t lhs_anti_offset;

            /** The mask to apply to the RHS of the boundary */
            uint64_t rhs_mask;

            /** The right shift to apply to the bits on the RHS of the boundary */
            uint64_t rhs_offset;

            /** The column which contains boundary. */
            size_t wrap_column;

        public:
            /** Calculate boundary information */
            explicit constexpr BoundaryCalculator(const size_t column_height) noexcept {
                if (0 != column_height) {
                    this->wrap_column = qubits_per_slide / column_height;
                    size_t left_qubits = qubits_per_slide %  column_height;
                    if (left_qubits > 0) {
                        // Unaligned case:
                        this->lhs_anti_offset = 2 * (qubits_per_slide - left_qubits);
                        this->lhs_mask = ~calculate_mask_from_bits(this->lhs_anti_offset);
                        this->rhs_offset = 2 * left_qubits;
                        this->rhs_mask = calculate_mask_from_qubits(column_height - left_qubits);
                    } else {
                        // Aligned case:
                        this->lhs_mask = 0;
                        this->lhs_anti_offset = std::numeric_limits<storage_t>::digits;
                        this->rhs_mask = calculate_mask_from_qubits(column_height);
                        this->rhs_offset = 0;
                    }
                } else {
                    // No columns
                    this->wrap_column = 1;
                    this->lhs_mask = 0;
                    this->lhs_anti_offset = 0;
                    this->rhs_mask = 0;
                    this->rhs_offset = 0;
                }
            }

            [[nodiscard]] constexpr inline storage_t evaluate(const Datum& input) const noexcept {
                return ((input[0] & this->lhs_mask) >> lhs_anti_offset) | ((input[1] & this->rhs_mask) << rhs_offset);
            }

            constexpr inline void splice_in(Datum& output, storage_t value) const noexcept {
                output[0] |= (value << lhs_anti_offset) & this->lhs_mask;
                output[1] |= (value >> rhs_offset) & this->rhs_mask;
            }
        } boundary_info;


        explicit constexpr SiteHasher(const size_t qubit_count, const size_t col_size = 0)
                : SiteHasherBase{qubit_count, col_size}, boundary_info{col_size} {
            assert(qubit_count <= 2*qubits_per_slide);
        }

        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] constexpr Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
            Datum output;
            output.fill(0);
            for (const auto op: sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                const storage_t slide_num = qubit_number / qubits_per_slide;
                const storage_t slide_offset = qubit_number % qubits_per_slide;
                output[slide_num] += ((pauli_op + 1) << (slide_offset * 2));
            }
            return output;
        }

        [[nodiscard]] inline constexpr Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            return this->hash(sequence);
        }

        /**
         * Reconstruct a sequence from its Pauli site hash.
         */
        [[nodiscard]] sequence_storage_t unhash(const Datum& input) const {

            // In principle reserve() could be called here; but since we almost always have shorter words than the stack
            // limit, in most use cases the time spend counting bits beforehand would result in a pessimization.
            sequence_storage_t output;

            // Cycle through slides, finding non-zero entries
            for (size_t slide = 0; slide < 2; ++slide) {
                storage_t qubit_number = slide * qubits_per_slide;
                storage_t within_slide_cursor = input[slide];
                while (0 != within_slide_cursor) {
                    storage_t qubit_offset = std::countr_zero(within_slide_cursor) / 2;
                    within_slide_cursor >>= (qubit_offset * 2); // Consume bits up to qubit

                    const oper_name_t pauli_op = (within_slide_cursor & 0x3) - 1; // 01 -> X (0), 10 -> Y (1), 11 -> Z (2);

                    qubit_number += qubit_offset;
                    output.emplace_back((qubit_number*3) + pauli_op);

                    // Consume qubit:
                    within_slide_cursor >>= 2;
                    qubit_number += 1;
                }
            }
            return output;
        }

        /**
         * Bit rotation for chain
         */
        [[nodiscard]] constexpr std::array<storage_t, 2>
        cyclic_shift(const std::array<storage_t, 2>& input, size_t offset) const noexcept {
            // Number of slides that have to be pushed around
            offset = (offset % this->qubits);

            // If no offset, copy input
            if (0 == offset) {
                return input;
            }

            // Otherwise, do shift
            std::array<storage_t, 2> output;

            // Right shift the start of input to end of output
            const bool flip_front = offset >= qubits_per_slide;
            if (!flip_front) {
                const size_t front_bit_offset = 2 * offset;
                const size_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;

                // Right shift start of input to end of output
                output[0] = input[0] << front_bit_offset;
                output[1] = (input[0] >> front_bit_anti_offset) | (input[1] << front_bit_offset);
            } else {
                const size_t front_bit_offset = (2 * (offset - qubits_per_slide)) - std::numeric_limits<storage_t>::digits;

                output[0] = 0;
                output[1] = (input[0] << front_bit_offset); // input[1] term is obliterated by large shift!
            }

            // Now do rightward shift
            const size_t back_offset = this->qubits - offset;
            const bool flip_back = back_offset >= qubits_per_slide;
            if (!flip_back) {
                const size_t back_bit_offset = back_offset * 2;
                const size_t back_bit_anti_offset = std::numeric_limits<storage_t>::digits - back_bit_offset;

                // Some overlap for second slide, as offset is small compared to remainder
                output[0] |= (input[1] << back_bit_anti_offset) | (input[0] >> back_bit_offset);
                output[1] |= (input[1] >> back_bit_offset);
            } else {
                const size_t back_bit_offset = (back_offset - qubits_per_slide) * 2;

                // Otherwise, we have wrapping behaviour. Nothing from input 0, as jump is big enough to skip.
                output[0] |= (input[1] >> back_bit_offset);
            }

            output[1] &= this->final_slide_mask;
            return output;
        }


        /**
         * Offset along major axis:
         */
        [[nodiscard]] inline constexpr Datum col_shift(const Datum input, const size_t offset) const noexcept {
            return cyclic_shift(input, (offset % this->row_width) * this->column_height);
        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] constexpr Datum row_shift(const Datum& input, size_t offset) const noexcept {

            offset = offset % this->column_height;
            if (0 == offset) {
                return input;
            }

            const storage_t bit_offset = 2 * offset;
            const storage_t bit_anti_offset = (this->column_height * 2) - bit_offset;

            // Prepare output
            Datum output{0, 0};

            // Left slide columns
            if (0 != input[0]) {
                for (size_t cIdx = 0; cIdx < this->boundary_info.wrap_column; ++cIdx) {
                    storage_t word = (input[0] >> (cIdx * this->column_height * 2)) & this->column_mask; // read col
                    word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask; // rot col
                    output[0] |= word << (cIdx * this->column_height * 2); // write col
                }
            }

            // Middle column
            storage_t middle_word = this->boundary_info.evaluate(input);
            middle_word = ((middle_word << bit_offset) | (middle_word >> bit_anti_offset)) & this->column_mask;
            this->boundary_info.splice_in(output, middle_word);

            // Right slide columns
            if (0 != input[1]) {
                for (size_t cIdx = this->boundary_info.wrap_column + 1; cIdx < this->row_width; ++cIdx) {
                    storage_t word = (input[1] >> ((cIdx * this->column_height * 2)
                                                   - std::numeric_limits<storage_t>::digits))
                                     & this->column_mask; // Read col
                    word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask; // rot col
                    output[1] |= word << ((cIdx * this->column_height * 2) - std::numeric_limits<storage_t>::digits);
                }
            }

            return output;
        }


        /**
         * Slice out value of a single column
         */
        [[nodiscard]] constexpr storage_t extract_column(const Datum& input, size_t column) const noexcept {
            assert(column < this->row_width);

            // Trivially on first page?
            if (column < this->boundary_info.wrap_column) {
                return (input[0] >> (column * this->column_height * 2)) & this->column_mask;
            } else if (column > this->boundary_info.wrap_column) {
                // Offset by remainder
                return (input[1] >> ((column * this->column_height * 2) - std::numeric_limits<storage_t>::digits))
                        & this->column_mask;
            }
            // Return column case
            return this->boundary_info.evaluate(input);
        }


    };



}