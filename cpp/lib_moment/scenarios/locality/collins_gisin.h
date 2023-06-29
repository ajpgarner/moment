/**
 * collins_gisin.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "dictionary/operator_sequence.h"

#include <shared_mutex>
#include <set>
#include <stdexcept>
#include <span>
#include <vector>

namespace Moment {
    class MatrixSystem;
    class SymbolTable;
}

namespace Moment::Locality {
    class LocalityContext;

    namespace errors {
        class BadCGError : public std::runtime_error {
        public:
            explicit BadCGError(const std::string& what) : std::runtime_error(what) { }
        };
    };

    class CollinsGisin {
    public:
        /** The associated operator context */
        const LocalityContext& context;

        /** The size of each dimension of the Collins Gisin (i.e. operators per party + 1) */
        const std::vector<size_t> Dimensions;

    private:
        std::vector<ptrdiff_t> real_indices;

        std::vector<OperatorSequence> sequences;

        std::vector<symbol_name_t> symbols;

        std::set<size_t> missing_symbols;

        mutable std::shared_mutex symbol_mutex;

    public:
        explicit CollinsGisin(const MatrixSystem& matrixSystem);

        /**
         * Throws a BadCGError if the supplied index is invalid.
         * @param index The Collins-Gisin index to check
         */
        void validate_index(std::span<const size_t> index) const;

        [[nodiscard]] size_t index_to_offset(std::span<const size_t> index) const;

        [[nodiscard]] inline size_t index_to_offset(std::initializer_list<size_t> index) const {
            std::vector<size_t> v(index);
            return index_to_offset(v);
        }

        [[nodiscard]] OperatorSequence index_to_sequence(std::span<const size_t> index) const;

        [[nodiscard]] inline OperatorSequence index_to_sequence(std::initializer_list<size_t> index) const {
            std::vector<size_t> v(index);
            return index_to_sequence(v);
        }


        [[nodiscard]] const std::vector<OperatorSequence>& Sequences() const noexcept { return this->sequences; }

        [[nodiscard]] const std::vector<symbol_name_t>& Symbols() const;

        [[nodiscard]] const std::vector<ptrdiff_t>& RealIndices() const;

        [[nodiscard]] bool HasSymbols() const noexcept;

        /**
         * Attempt to find all missing symbol IDs.
         * @return True if all symbols are now filled.
         */
        bool fill_missing_symbols(const SymbolTable& symbol_table) noexcept;

    };

}