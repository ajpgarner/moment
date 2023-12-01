/**
 * site_hasher.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"

#include <cassert>

#include <array>
#include <bit>
#include <bitset>
#include <concepts>
#include <limits>
#include <span>
#include <vector>

namespace Moment::Pauli {


    template <int Slides, std::unsigned_integral storage_t = uint64_t>
    class SiteHasher {
    public:
        static_assert(Slides > 0);
        using Datum = std::array<storage_t, Slides>;
        /** The number of qubits we can fit on each slide */
        constexpr static oper_name_t QubitsPerSlide = sizeof(storage_t) * 4; // should be 32

        /** Maximum number of allowed slides by hasher. */
        constexpr static const size_t slides = Slides;

        /** Number of qubits in this particular hasher instance. */
        const size_t qubits;

        /** Number of qubits on the final slide of this hasher. */
        const size_t qubits_on_final_slide;

        /** The mask for final slide when rotating */
        const storage_t mask;



        /**
         * Construct a site-hasher
         * @param qubit_count The maximum number of qubits in the hasher
         */
        explicit constexpr SiteHasher(const size_t qubit_count)
                : qubits{qubit_count},
                  qubits_on_final_slide{calculate_last_slide_qubit_count(qubit_count)},
                  mask{calculate_mask_from_last_slide_qubits(qubits_on_final_slide)} {
            static_assert(slides > 1);
            assert(this->qubits <= QubitsPerSlide * Slides);
        }

        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] constexpr Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            Datum output;
            output.fill(0);
            for (const auto op: sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                const storage_t slide_num = qubit_number / QubitsPerSlide;
                const storage_t slide_offset = qubit_number % QubitsPerSlide;
                output[slide_num] += ((pauli_op + 1) << (slide_offset * 2));
            }
            return output;
        }


        /**
         * Shift the data about in a chain
         */
        [[nodiscard]] constexpr inline Datum cyclic_shift(const Datum& input, size_t offset) const noexcept {
            return this->do_cyclic_shift<Slides>(input, offset);
        }

    private:
        /**
         * Calculates the bit mask for final page
         */
        [[nodiscard]] constexpr static storage_t calculate_mask_from_last_slide_qubits(const size_t ls_qubits) noexcept {
            if ((QubitsPerSlide == ls_qubits) || (0 == ls_qubits)) {
                return ~static_cast<storage_t>(0); // 0xff...ff
            }

            // e.g (1<<63)=0x80...; -1 sets all but the highest bit.
            return static_cast<storage_t>((static_cast<storage_t>(1) << (ls_qubits* 2)) - 1);
        }


        /**
         * Calculates number of qubits on last slide
         */
        [[nodiscard]] constexpr static size_t
        calculate_last_slide_qubit_count(const std::make_signed<size_t>::type qubit_count) noexcept {
            return ((qubit_count - 1)) % (std::numeric_limits<storage_t>::digits/2) + 1;
        }

        /**
         * General case of bit rotation:
         */
        template<size_t ActualSlides>
        constexpr std::array<storage_t, ActualSlides>
        do_cyclic_shift(const std::array<storage_t, ActualSlides>& input, size_t offset) const noexcept {
            static_assert(std::is_same<std::array<storage_t, ActualSlides>, Datum>::value);

            // Number of slides that have to be outright pushed around
            offset = offset % this->qubits;

            // If no offset, copy input
            if (0 == offset) {
                return input;
            }

            // Calculate offset parameters
            const size_t front_slide_offset = offset / QubitsPerSlide; // Positive offset to slides
            const size_t front_bit_offset = (offset % QubitsPerSlide) * 2; // 2 bits per qubit
            const size_t back_offset = (this->qubits - offset);
            const size_t back_slide_offset = back_offset / QubitsPerSlide;
            const size_t back_bit_offset = (back_offset % QubitsPerSlide) * 2;
            const size_t back_bit_anti_offset = std::numeric_limits<storage_t>::digits - back_bit_offset;

            // Go through input slides and distribution
            Datum output;
            output.fill(0);

            // Right shift start of word to end
            if (0 == front_bit_offset) {
                for (size_t idx = front_slide_offset; idx < Slides; ++idx) {
                    output[idx] = input[idx-front_slide_offset];
                }
            } else {
                const size_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;
                output[front_slide_offset] = input[0] << front_bit_offset;
                for (size_t idx = front_slide_offset+1; idx < Slides; ++idx) {
                    output[idx] = (input[idx-front_slide_offset] << front_bit_offset)
                                    + (input[idx-front_slide_offset - 1] >> front_bit_anti_offset);
                }
            }

            // Left shift end of word to beginning of output
            if (0 == back_bit_offset) {
                for (ptrdiff_t idx = 0; idx < Slides - back_slide_offset; ++idx) {
                    output[idx] |= input[idx + back_slide_offset];
                }
            } else {
                 // otherwise 0 offset?
                for (ptrdiff_t idx = 0; idx < Slides - back_slide_offset - 1; ++idx) {
                    output[idx] |= (input[idx + back_slide_offset] >> back_bit_offset)
                                 | (input[idx + back_slide_offset+1] << back_bit_anti_offset);
                }
                output[Slides - back_slide_offset - 1] |= (input[Slides - 1] >> back_bit_offset);
            }

            // Apply mask to final slide
            output[Slides-1] &= this->mask;

            return output;

        }

        /**
         * Specialization of bit rotation for pair (i.e. up to 64 qubits).
         */
        template<>
        constexpr std::array<storage_t, 2>
        do_cyclic_shift(const std::array<storage_t, 2>& input, size_t offset) const noexcept {
            static_assert(std::is_same<std::array<storage_t, 2>, Datum>::value);

            // Number of slides that have to be outright pushed around
            offset = (offset % this->qubits);

            // If no offset, copy input
            if (0 == offset) {
                return input;
            }

            // Otherwise, do shift
            std::array<storage_t, 2> output;

            // Right shift the start of input to end of output
            const bool flip_front = offset >= QubitsPerSlide;
            if (!flip_front) {
                const size_t front_bit_offset = 2 * offset;
                const size_t front_bit_anti_offset = std::numeric_limits<storage_t>::digits - front_bit_offset;

                // Right shift start of input to end of output
                output[0] = input[0] << front_bit_offset;
                output[1] = (input[0] >> front_bit_anti_offset) | (input[1] << front_bit_offset);
            } else {
                const size_t front_bit_offset = (2 * (offset - QubitsPerSlide)) - std::numeric_limits<storage_t>::digits;

                output[0] = 0;
                output[1] = (input[0] << front_bit_offset); // input[1] term is obliterated by large shift!
            }

            // Now do rightward shift
            const size_t back_offset = this->qubits - offset;
            const bool flip_back = back_offset >= QubitsPerSlide;
            if (!flip_back) {
                const size_t back_bit_offset = back_offset * 2;
                const size_t back_bit_anti_offset = std::numeric_limits<storage_t>::digits - back_bit_offset;

                // Some overlap for second slide, as offset is small compared to remainder
                output[0] |= (input[1] << back_bit_anti_offset) | (input[0] >> back_bit_offset);
                output[1] |= (input[1] >> back_bit_offset);
            } else {
                const size_t back_bit_offset = (back_offset - QubitsPerSlide) * 2;

                // Otherwise, we have wrapping behaviour. Nothing from input 0, as jump is big enough to skip.
                output[0] |= (input[1] >> back_bit_offset);
            }

            output[1] &= this->mask;
            return output;
        }
    };


    /**
     * Specialization of site-hasher for 16 or fewer qubits, that can be hashed into a single number.
     */
    template<std::unsigned_integral storage_t>
    class SiteHasher<1, storage_t> {
    public:
        using Datum = storage_t;
        constexpr static oper_name_t QubitsPerSlide = sizeof(storage_t) * 4; // should be 32

        /** Maximum number of allowed slides in hasher template. */
        const size_t qubits;

        /** The mask for slide when rotating */
        const storage_t mask;

        /** Maximum number of slides allowed by hasher template (here, 1). */
        constexpr static const size_t slides = 1;


        /**
         * Construct a site-hasher
         * @param qubit_count The maximum number of qubits in the hasher.
         */
        explicit constexpr SiteHasher(const size_t qubit_count)
            : qubits{qubit_count}, mask{make_mask(qubit_count)} {

        }

        /**
         * Hash the data from an operator sequence into a Pauli site hash.
         * Nominally is a monotonic function on operator's own hash.
         */
        [[nodiscard]] constexpr Datum operator()(const std::span<const oper_name_t> sequence) const noexcept {
            Datum output{0};  // Prepare all zero output
            for (const auto op : sequence) {
                const storage_t qubit_number = op / 3;
                const storage_t pauli_op = (op % 3); // I=00, X=01, Y=10, Z=11
                output += ((pauli_op+1) << (qubit_number*2));
            }
            return output;
        }

        /**
         * Shift the data in a chain
         */
        [[nodiscard]] constexpr Datum cyclic_shift(Datum initial_value, const size_t offset) const noexcept {
            assert(offset <= QubitsPerSlide);
            if (QubitsPerSlide == this->qubits) {
                return std::rotl(initial_value, 2 * offset);
            } else {
                return ((initial_value << (2*offset)) & this->mask)
                    | (initial_value >> ((this->qubits - offset) * 2));
            }
        }

        /**
         * Calculates a bit mask
         */
        [[nodiscard]] constexpr static storage_t make_mask(const size_t qubit_count) noexcept {
            const auto remainder = (2*qubit_count) % std::numeric_limits<storage_t>::digits;
            if (0 == remainder) {
                return ~static_cast<storage_t>(0); // 0xff...ff
            }
            // e.g (1<<63)=0x80...; -1 sets all but the highest bit.
            return (static_cast<storage_t>(1) << (remainder)) - 1;
        }


    };



}