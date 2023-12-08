/**
 * site_hasher_impl.h
 *
 * Implementation by bit-field of the translational equivalence class detection.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_simplifier.h"

#include <cassert>

#include <array>
#include <bit>
#include <concepts>
#include <limits>

namespace Moment::Pauli {
    /**
     * Common information for implementation of site hasher.
     */
    class SiteHasherImplBase : public MomentSimplifier {
    public:
        using storage_t = uint64_t;

        /** The number of qubits we can fit on each slide */
        constexpr static storage_t qubits_per_slide = sizeof(storage_t) * 4ULL; // should be 32

        /** Number of qubits on the final slide of this hasher. */
        const storage_t qubits_on_final_slide;

        /** The mask for final slide when rotating */
        const storage_t final_slide_mask;

        /** The mask for a single column (max size, 32 qubits). */
        const storage_t column_mask;

    protected:
        /**
         * Construct a site-hasher
         * @param qubit_count The maximum number of qubits in the hasher.
         * @param col_size The number of qubits in a column.
         */
        explicit SiteHasherImplBase(const PauliContext& context, const uint64_t label)
            : MomentSimplifier{context, label}, qubits_on_final_slide{calculate_last_slide_qubit_count(qubits)},
              final_slide_mask{calculate_mask_from_qubits(qubits_on_final_slide)},
              column_mask(calculate_mask_from_qubits(column_height)) { }

    protected:
        /**
         * Calculates number of qubits on last slide
         */
        [[nodiscard]] constexpr static size_t
        calculate_last_slide_qubit_count(const std::make_signed<size_t>::type qubit_count) noexcept {
            return ((qubit_count - 1)) % (std::numeric_limits<storage_t>::digits/2) + 1;
        }

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
        [[nodiscard]] constexpr inline static storage_t
        calculate_mask_from_qubits(const size_t num_qubits) noexcept {
            return calculate_mask_from_bits(num_qubits*2);
        }
    };
    
    
    /**
      * General hasher implementation, for arbitrary number of qubits, but maximum column size of 16.
      * @tparam num_slides
      */
    template<size_t num_slides>
    class SiteHasherSized : public SiteHasherImplBase {
    public:
        /** Hash result. */
        using Datum = std::array<storage_t, num_slides>;

        /** Maximum number of allowed slides by hasher. */
        constexpr static const size_t slides = num_slides;

    protected:
        explicit SiteHasherSized(const PauliContext& context)
                : SiteHasherImplBase{context, num_slides} {
            assert(qubits <= slides * qubits_per_slide);
        }

    public:

        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
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

        /**
         * Gets the hash of an empty string
         */
        [[nodiscard]] constexpr inline static Datum empty_hash() noexcept {
            Datum output;
            output.fill(0);
            return output;
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

                    const oper_name_t pauli_op =
                            (within_slide_cursor & 0x3) - 1; // 01 -> X (0), 10 -> Y (1), 11 -> Z (2);

                    qubit_number += qubit_offset;
                    output.emplace_back((qubit_number * 3) + pauli_op);

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
        [[nodiscard]] Datum cyclic_shift(const Datum& input, size_t offset) const noexcept {
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
                    output[idx] = input[idx - front_slide_offset];
                }
            } else {
                const size_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;
                output[front_slide_offset] = input[0] << front_bit_offset;
                for (size_t idx = front_slide_offset + 1; idx < slides; ++idx) {
                    output[idx] = (input[idx - front_slide_offset] << front_bit_offset)
                                  + (input[idx - front_slide_offset - 1] >> front_bit_anti_offset);
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
                                   | (input[idx + back_slide_offset + 1] << back_bit_anti_offset);
                }
                output[slides - back_slide_offset - 1] |= (input[slides - 1] >> back_bit_offset);
            }

            // Apply mask to final slide
            output[slides - 1] &= this->final_slide_mask;

            return output;

        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] Datum row_cyclic_shift(const Datum& input, size_t offset) const noexcept {

            // Normalize rotation, and get within-column offsets
            offset = offset % this->row_width;
            const storage_t bit_offset = 2ULL * offset;
            const storage_t bit_anti_offset = (this->column_height * 2ULL) - bit_offset;

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
                                     | ((input[first_slide + 1] & remainder_mask) << (remainder * 2)); // Read word
                    word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask; // Rotate word

                    output[first_slide] |= (word << offset_slide_one);  // Add word part to first slide
                    output[first_slide + 1] = (word
                            >> (remainder * 2));  // Overwrite second slide with  second word part

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
        [[nodiscard]] storage_t extract_column(const Datum& input, size_t column) const noexcept {
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
                output |= ((input[first_slide + 1] & remainder_mask) << (remainder * 2));
            }

            return output;
        }

        [[nodiscard]] constexpr static bool less(const Datum& lhs, const Datum& rhs) noexcept {
            for (ptrdiff_t idx = num_slides - 1; idx >= 0; --idx) {
                if (lhs[idx] < rhs[idx]) {
                    return true;
                } else if (lhs[idx] > rhs[idx]) {
                    return false;
                }
            }
            return false;
        }

    };

/**
 * Specialized hasher for up to 32 qubits, stored in one 64-bit number
 */
    template<>
    class SiteHasherSized<1> : public SiteHasherImplBase {
    public:
        /** Hash result type. */
        using Datum = storage_t;

        /** Number of slides. */
        constexpr static size_t slides = 1;

    protected:
        /**
         * Construct a Pauli site hasher for up to 32 qubits
         * @param qubit_count
         * @param col_size The number of qubits per column for a lattice, or set to 0 for a chain.
         */
        explicit SiteHasherSized(const PauliContext& context) : SiteHasherImplBase{context, 1} {
            assert(qubits <= qubits_per_slide);
        }

    public:
        /**
         * Hash: Specialization to 1 slide (up to 32 qubits).
         */
        [[nodiscard]] Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
            // Prepare all zero output
            Datum output{0};

            for (const auto op: sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                output += ((pauli_op + 1) << (qubit_number * 2));
            }
            return output;
        }


        /**
         * Gets the hash of an empty string
         */
        [[nodiscard]] constexpr inline static Datum empty_hash() noexcept {
            return Datum{0};
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
                output.emplace_back((qubit_number * 3) + pauli_op);

                // Consume qubit:
                within_slide_cursor >>= 2;
                qubit_number += 1;
            }
            return output;
        }


        /**
         * Bit rotation for chain
         */
        [[nodiscard]] Datum cyclic_shift(const Datum input, size_t offset) const noexcept {
            offset = 2 * (offset % qubits_per_slide);
            if (0 == offset) {
                return input;
            }
            return Datum{((input << offset) & this->final_slide_mask) | (input >> (2 * this->qubits - offset))};
        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] Datum row_cyclic_shift(const Datum input, size_t offset) const noexcept {
            assert(this->column_height != 0);
            offset = offset % this->column_height;
            if (0 == offset) {
                return input;
            }

            const storage_t bit_offset = 2ULL * offset;
            const storage_t bit_anti_offset = (this->column_height * 2ULL) - bit_offset;

            Datum output{0};
            for (size_t cIdx = 0; cIdx < this->row_width; ++cIdx) { // Cycle through columns
                storage_t word = this->extract_column(input, cIdx);
                word = ((word << bit_offset) | (word >> bit_anti_offset)) & this->column_mask;
                output |= word << (cIdx * this->column_height * 2);
            }
            return output;
        }

        /**
         * Slice out hash page from a single column.
         */
        [[nodiscard]] storage_t extract_column(const Datum input, size_t column) const noexcept {
            assert(column < this->row_width);
            return ((input >> (column * this->column_height * 2)) & this->column_mask);
        }

        [[nodiscard]] constexpr inline static bool less(const Datum lhs, const Datum rhs) noexcept {
            return lhs < rhs;
        }

    };

    /**
     * Specialized hasher for between 33 and 64 qubits (inclusive), stored in two 64-bit numbers.
     */
    template<>
    class SiteHasherSized<2> : public SiteHasherImplBase {
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
                    size_t left_qubits = qubits_per_slide % column_height;
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

    protected:
        explicit SiteHasherSized(const PauliContext& context)
            : SiteHasherImplBase{context, 2},
                boundary_info{column_height} {
            assert(qubits > qubits_per_slide);
            assert(qubits <= 2 * qubits_per_slide);
        }

    public:
        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] Datum hash(const std::span<const oper_name_t> sequence) const noexcept {
            Datum output{0, 0};
            for (const auto op: sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                const storage_t slide_num = qubit_number / qubits_per_slide;
                const storage_t slide_offset = qubit_number % qubits_per_slide;
                output[slide_num] += ((pauli_op + 1) << (slide_offset * 2));
            }
            return output;
        }

        /**
         * Gets the hash of an empty string
         */
        [[nodiscard]] constexpr inline static Datum empty_hash() noexcept {
            return Datum{0, 0};
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

                    const oper_name_t pauli_op =
                            (within_slide_cursor & 0x3) - 1; // 01 -> X (0), 10 -> Y (1), 11 -> Z (2);

                    qubit_number += qubit_offset;
                    output.emplace_back((qubit_number * 3) + pauli_op);

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
        [[nodiscard]] Datum cyclic_shift(const Datum& input, size_t offset) const noexcept {
            // Number of slides that have to be pushed around
            offset = (offset % this->qubits);

            // If no offset, copy input
            if (0 == offset) {
                return input;
            }

            // Otherwise, do shift
            Datum output;

            // Right shift the start of input to end of output
            if (offset < qubits_per_slide) {
                const uint64_t front_bit_offset = 2ULL * offset;
                const uint64_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;

                // Right shift start of input to end of output
                output[0] = input[0] << front_bit_offset;
                output[1] = (input[0] >> front_bit_anti_offset) | (input[1] << front_bit_offset);
            } else {
                const uint64_t front_bit_offset = 2ULL * (offset - qubits_per_slide);

                output[0] = 0;
                output[1] = (input[0] << front_bit_offset); // input[1] term is obliterated by large shift!
            }

            // Now do rightward shift
            const uint64_t back_offset = this->qubits - offset;
            if (back_offset < qubits_per_slide) {
                const uint64_t back_bit_offset = back_offset * 2;
                const uint64_t back_bit_anti_offset = std::numeric_limits<storage_t>::digits - back_bit_offset;

                // Some overlap for second slide, as offset is small compared to remainder
                output[0] |= (input[1] << back_bit_anti_offset) | (input[0] >> back_bit_offset);
                output[1] |= (input[1] >> back_bit_offset);
            } else {
                const uint64_t back_bit_offset = 2ULL * (back_offset - qubits_per_slide);

                // Otherwise, we have wrapping behaviour. Nothing from input 0, as jump is big enough to skip.
                output[0] |= (input[1] >> back_bit_offset);
            }

            output[1] &= this->final_slide_mask;
            return output;
        }

        /**
         * Offset along minor axis:
         */
        [[nodiscard]] Datum row_cyclic_shift(const Datum& input, size_t offset) const noexcept {

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
        [[nodiscard]] storage_t extract_column(const Datum& input, size_t column) const noexcept {
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


        [[nodiscard]] constexpr inline static bool less(const Datum& lhs, const Datum& rhs) noexcept {
            if (lhs[1] < rhs[1]) {
                return true;
            } else if (lhs[1] > rhs[1]) {
                return false;
            }
            return lhs[0] < rhs[0];
        }
    };


    /**
     * Final hasher implementation, with common functions.
     * @tparam num_slides Number of slides in implementation
     */
    template<size_t num_slides>
    class SiteHasher final : public SiteHasherSized<num_slides> {
    public:
        using Datum = typename SiteHasherSized<num_slides>::Datum;

    public:
        explicit SiteHasher(const PauliContext& context)
                : SiteHasherSized<num_slides>{context} { }

        /**
         * Rotate around columns (i.e. major-axis shift).
         */
        [[nodiscard]] inline Datum col_shift(const Datum& input, const size_t offset) const noexcept {
            return SiteHasherSized<num_slides>::cyclic_shift(input, (offset % this->row_width) * this->column_height);
        }

        /**
         * Alias for hash function.
         */
        [[nodiscard]] inline Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            return this->hash(sequence);
        }

        /**
         * Lattice shift.
         * @param input Input hash value.
         * @param row_offset Rotation within columns (offsets row number of each operator)
         * @param col_offset Rotation within rows (offsets col number of each operator)
         * @return Shifted hash value.
         */
        [[nodiscard]] inline Datum
        lattice_shift(const Datum& input, const size_t row_offset, const size_t col_offset) const noexcept {
            return SiteHasherSized<num_slides>::row_cyclic_shift(col_shift(input, col_offset), row_offset);
        }

        /**
         * Gets the equivalence class hash and current hash of an operator sequence.
         * The equivalence value is not the strict minimum over all translations, but over all translations such that
         * one qubit aligns with lattice position [0,0].
         * @param sequence The sequence to hash and translate.
         * @return Pair; first: with equivalence value of hash after shifts, second: hash of original sequence
         */
        [[nodiscard]] inline std::pair<Datum, Datum>
        canonical_hash(const std::span<const oper_name_t> sequence) const noexcept {
            return (this->row_width == 1) ? do_canonical_hash<false>(sequence)
                                          : do_canonical_hash<true>(sequence);
        }

        using MomentSimplifier::canonical_sequence;

        [[nodiscard]] sequence_storage_t canonical_sequence(const std::span<const oper_name_t> input) const final {
            // Find equivalence class
            const auto [smallest_hash, actual_hash] = canonical_hash(input);

            // Operator sequence is already minimal
            if (smallest_hash == actual_hash) {
                // Copy input to output:
                sequence_storage_t output;
                output.reserve(input.size());
                std::copy(input.begin(), input.end(), std::back_inserter(output));
                return output;
            } else {
                // Otherwise, reconstruct operator sequence data from minimal hash value:
                return this->unhash(smallest_hash);
            }
        }


        /**
         * Tests canonical version of operator sequence
         */
        [[nodiscard]] bool is_canonical(const std::span<const oper_name_t> input) const noexcept final {
            // Find equivalence class
            const auto [smallest_hash, actual_hash] = canonical_hash(input);

            // Is input operator sequence already minimal?
            return (smallest_hash != actual_hash);
        }


    private:
        template<bool is_lattice_mode>
        [[nodiscard]] std::pair <Datum, Datum>
        do_canonical_hash(const std::span<const oper_name_t> sequence) const noexcept {
            // Empty sequence is always hash 0:
            if (sequence.empty()) {
                return {SiteHasherSized<num_slides>::empty_hash(), SiteHasherSized<num_slides>::empty_hash()};
            }

            // First, calculate hash of supplied sequence
            std::pair <Datum, Datum> output; // first remains uninitialized...
            output.second = this->hash(sequence);

            // Now, try offsetting each element
            bool done_once = false;
            for (const oper_name_t oper: sequence) {
                const oper_name_t qubit_number = oper / 3;
                if constexpr (is_lattice_mode) {
                    // lattice offset
                    const oper_name_t col_number = qubit_number / this->column_height;
                    const oper_name_t row_number = qubit_number % this->column_height;
                    const size_t col_shift = this->row_width - col_number;
                    const size_t row_cyclic_shift = this->column_height - row_number;

                    Datum candidate_hash = this->lattice_shift(output.second, row_cyclic_shift, col_shift);
                    if (!done_once || SiteHasherSized<num_slides>::less(candidate_hash, output.first)) {
                        output.first = candidate_hash;
                        done_once = true;
                    }

                } else {
                    // chain offset
                    const size_t shift = this->qubits - qubit_number;
                    Datum candidate_hash = this->cyclic_shift(output.second, shift);
                    if (!done_once || SiteHasherSized<num_slides>::less(candidate_hash, output.first)) {
                        output.first = candidate_hash;
                        done_once = true;
                    }
                }
            }
            return output;
        }

    };
}