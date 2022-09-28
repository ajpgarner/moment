/**
 * collins_gisin.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol.h"
#include "../operator_sequence.h"

#include <stdexcept>
#include <span>
#include <vector>

namespace NPATK {
    class MatrixSystem;
    class Context;

    namespace errors {
        class BadCGError : std::runtime_error {
        public:
            explicit BadCGError(const std::string& what) : std::runtime_error(what) { }
        };
    };

    class CollinsGisin {
    public:
        /** The associated operator context */
        const Context& context;

        /** The size of each dimension of the Collins Gisin (i.e. operators per party + 1) */
        const std::vector<size_t> Dimensions;

    private:
        std::vector<ptrdiff_t> real_indices;

        std::vector<OperatorSequence> sequences;

        std::vector<symbol_name_t> symbols;

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

        [[nodiscard]] const std::vector<ptrdiff_t>& RealIndices() const noexcept { return this->real_indices; }

        [[nodiscard]] const std::vector<OperatorSequence>& Sequences() const noexcept { return this->sequences; }

        [[nodiscard]] const std::vector<symbol_name_t>& Symbols() const noexcept { return this->symbols; }

    };

}