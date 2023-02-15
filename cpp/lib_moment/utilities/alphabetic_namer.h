/**
 * alphabetic_namer.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include <string>

namespace Moment {

    /**
     * Utility functor, for mapping numeric IDs to the alphabet.
     */
    class AlphabeticNamer {
    private:
        const bool upper_case;

    public:
        constexpr explicit AlphabeticNamer(bool is_upper_case = true) noexcept : upper_case{is_upper_case} { }

        /**
         * Assigns an alphabetic name from the supplied id, in the 'Excel-like' order A-Z, AA-ZZ, AAA-ZZZ etc.
         */
        std::string operator()(size_t id) const;


        /**
         * Assigns an alphabetic name from the supplied id, in the 'Excel-like' order A-Z, AA-ZZ, AAA-ZZZ etc.
         * @param index The index to find a name for
         * @param upper_case Whether the name should be upper or lowercase.
         * @return The std::string name.
         */
        inline static std::string index_to_name(size_t index, bool upper_case = true) {
            AlphabeticNamer namer{upper_case};
            return namer(index);
        }

        /**
         * Calculate the length of name required, using 'Excel-like' naming (a-z, aa-zz, aaa-zzz, etc.). The id is zero-
         * indexed, such that 0 => a.
         * @param id the paramter
         * @return the number of characters required to represent the name.
         */
        static size_t strlen(size_t id) noexcept;

        /**
         * Calculate the first index associated with strings of each new length; 0 => 0, 1 => 26, 2 => 26+26^2, etc.
         * @param level the string length minus one.
         */
        static size_t level_offset(size_t level) noexcept;
    };

}
