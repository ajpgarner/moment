/**
 * localizing_matrix_index.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "dictionary/operator_sequence.h"

#include <iosfwd>
#include <string>

namespace Moment {

    class Context;
    class SymbolTable;
    class MatrixSystem;

    struct LocalizingMatrixIndex {
    public:
        using OSGIndex = size_t;

    public:
        size_t Level;
        OperatorSequence Word;
        size_t WordHash;

    public:
        LocalizingMatrixIndex(size_t level, OperatorSequence word)
                : Level{level}, Word{std::move(word)},
                  WordHash{Word.hash()} {
        }

        constexpr bool operator<(const LocalizingMatrixIndex& rhs) const noexcept {
            // Order first by Level...
            if (this->Level < rhs.Level) {
                return true;
            } else if (this->Level > rhs.Level) {
                return false;
            }

            // Then by sequence hash...
            return (this->WordHash < rhs.WordHash);
        }

        /**
         * Gets part of index that can be used to call for an OSG associated with localizing matrix.
         */
        [[nodiscard]] constexpr inline friend size_t get_osg_index(const LocalizingMatrixIndex& lmi) {
            return lmi.Level;
        }

        friend std::ostream& operator<<(std::ostream& os, const LocalizingMatrixIndex& lmi);

        [[nodiscard]] std::string to_string() const;

        [[nodiscard]] std::string to_string(const Context& context) const;

        [[nodiscard]] std::string to_string(const MatrixSystem& matrix_system) const ;

    };

}