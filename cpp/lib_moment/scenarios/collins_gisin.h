/**
 * collins_gisin.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "dictionary/operator_sequence.h"

#include "utilities/multi_dimensional_index_iterator.h"
#include "utilities/tensor.h"

#include <shared_mutex>
#include <set>
#include <stdexcept>
#include <span>
#include <vector>

namespace Moment {
    class MatrixSystem;
    class SymbolTable;
    class Context;
    class ProbabilityTensor;

    namespace errors {
        class BadCGError : public std::runtime_error {
        public:
            explicit BadCGError(const std::string& what) : std::runtime_error(what) { }
        };
    };

    class CollinsGisinIterator;
    class CollinsGisinRange;
    using CollinsGisinIndex = Tensor::Index;
    using CollinsGisinIndexView = Tensor::IndexView;

    /**
     * Collins-Gisin tensor: an indexing scheme for real-valued operators that correspond to measurement outcomes.
     */
    class CollinsGisin : public Tensor {
    protected:
        struct GlobalMeasurementIndex {
            /** Which dimension of tensor does this measurement correspond to */
            size_t party;
            /** How far into this dimension is this measurement? */
            size_t offset;
            /** How many operators are defined by this measurement? */
            size_t length;

            GlobalMeasurementIndex() = default;
            GlobalMeasurementIndex(size_t p, size_t o, size_t l) : party{p}, offset{o}, length{l} { }
        };


    protected:
        std::vector<ptrdiff_t> real_indices;

        std::vector<OperatorSequence> sequences;

        std::vector<symbol_name_t> symbols;

        std::set<size_t> missing_symbols;

        std::vector<GlobalMeasurementIndex> gmIndex;

    private:
        mutable std::shared_mutex symbol_mutex;

    protected:
        /**
         * Construct Collins-Gisin tensor object.
         * @param dimensions The tensor's dimensions.
         */
        explicit CollinsGisin(std::vector<size_t>&& dimensions);

        /**
         * Attempt to populate symbol IDs for first time, and identify which are still missing.
         */
        void do_initial_symbol_search(const SymbolTable& symbols);

    public:
        /**
         * Attempt to find all missing symbol IDs.
         * @return True if all symbols are now filled.
         */
        bool fill_missing_symbols(const SymbolTable& symbol_table) noexcept;

    public:

        /**
         * Gets all operator sequences in tensor.
         */
        [[nodiscard]] const std::vector<OperatorSequence>& Sequences() const noexcept { return this->sequences; }

        /**
         * Gets indexed operator sequence
         * @param index Collins-Gisin index to operator sequence
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] const OperatorSequence& Sequences(CollinsGisinIndexView index) const {
            return this->Sequences()[index_to_offset(index)];
        }

        /**
         * Gets all symbols.
         * @throws BadCGError If any operators have not yet been identified in symbol table.
         */
        [[nodiscard]] const std::vector<symbol_name_t>& Symbols() const;

        /**
         * Gets indexed symbol ID.
         * @param index Collins-Gisin index to operator.
         * @throws BadCGError If index is invalid.
         * @throws BadCGError If op sequence at index has not been identified in symbol table.
         */
        [[nodiscard]] symbol_name_t Symbols(CollinsGisinIndexView index) const;

        /**
         * Gets all real basis elements.
         * @throws BadCGError If any operators have not yet been identified in symbol table.
         */
        [[nodiscard]] const std::vector<ptrdiff_t>& RealIndices() const;

        /**
         * Gets indexed real basis element
         * @param index Collins-Gisin index to operator.
         * @throws BadCGError If index is invalid.
         * @throws BadCGError If op sequence at index has not been identified in symbol table.
         */
        [[nodiscard]] ptrdiff_t RealIndices(CollinsGisinIndexView index) const;

        /**
         * True if every symbol in tensor has been identified.
         */
        [[nodiscard]] bool HasAllSymbols() const noexcept;

        /**
         * Translates Storage index in tensor to Collins-Gisin index.
         */
        [[nodiscard]] CollinsGisinIndex offset_to_index(size_t offset) const;

        /**
         * Get offset of all operators belonging to a supplied set of (global) measurement indices.
         * Pair: First gives CG index of first outcome, second gives extent in each dimension.
         * @return Iterator over identified range.
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const size_t> mmtIndices) const;

        /**
         * Get offset of all operators corresponding to supplied set of (global) measurement indices, fixing some of the
         * measurement outcomes.
         * @param mmtIndices A sorted list of global indices of the measurement.
         * @param fixedOutcomes List of outcome indices, or -1 if not fixed.
         * @return Iterator over identified range.
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const size_t> mmtIndices,
                                                             std::span<const oper_name_t> fixedOutcomes) const;


    public:
        friend class CollinsGisinIterator;
        friend class ProbabilityTensor;

    };

}