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

    class CollinsGisin;
    class CollinsGisinIterator;
    class CollinsGisinRange;
    using CollinsGisinIndex = Tensor::Index;
    using CollinsGisinIndexView = Tensor::IndexView;

    /** The number of elements, below which we cache the CG tensor explicitly. */
    constexpr static const size_t CG_explicit_element_limit = 1024ULL;

    struct CollinsGisinEntry {
    public:
        OperatorSequence sequence;

        symbol_name_t symbol_id = -1;

        ptrdiff_t real_index = -1;

    public:
        /**
         * Make operator sequence.
         * No bounds checks are done on the index.
         */
        explicit CollinsGisinEntry(const CollinsGisin& cgt, CollinsGisinIndexView index);

        bool find(const SymbolTable& table) noexcept;

        void find_or_fail(const SymbolTable& table);

    };

    /**
     * Collins-Gisin tensor: an indexing scheme for real-valued operators that correspond to measurement outcomes.
     */
    class CollinsGisin : public AutoStorageTensor<CollinsGisinEntry, CG_explicit_element_limit> {

    public:
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

        struct DimensionInfo {
            std::vector<oper_name_t> op_ids;
        };

    public:
        const Context& context;

        const SymbolTable& symbol_table;

    protected:

        /** Map from global measurement numbers to tensor indices. */
        std::vector<GlobalMeasurementIndex> gmIndex;

        /** Information about each dimension. */
        std::vector<DimensionInfo> dimensionInfo;

        /** Cached: missing real symbols. */
        std::set<size_t> missing_symbols;

    private:
        mutable std::shared_mutex symbol_mutex;

    protected:
        /**
         * Construct Collins-Gisin tensor object.
         * @param dimensions The tensor's dimensions.
         */
        explicit CollinsGisin(const Context& context, const SymbolTable& symbol_table,
                              std::vector<size_t>&& dimensions,
                              TensorStorageType storage = TensorStorageType::Automatic);

        /**
         * Attempt to populate symbol IDs for first time, and identify which are still missing.
         */
        void do_initial_symbol_search();

    public:
        /**
         * Attempt to find all missing symbol IDs.
         * @return True if all symbols are now filled.
         */
        bool fill_missing_symbols() noexcept;

    public:

        /**
         * Gets indexed operator sequence
         * @param index Collins-Gisin index to operator sequence
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] OperatorSequence Sequence(CollinsGisinIndexView index) const;

        /**
         * Gets indexed symbol ID.
         * @param index Collins-Gisin index to operator.
         * @throws BadCGError If index is invalid.
         * @throws BadCGError If op sequence at index has not been identified in symbol table.
         */
        [[nodiscard]] symbol_name_t Symbol(CollinsGisinIndexView index) const;

        /**
         * Gets indexed real basis element
         * @param index Collins-Gisin index to operator.
         * @throws BadCGError If index is invalid.
         * @throws BadCGError If op sequence at index has not been identified in symbol table.
         */
        [[nodiscard]] ptrdiff_t RealIndex(CollinsGisinIndexView index) const;

        /**
         * True if every symbol in tensor has been identified.
         */
        [[nodiscard]] bool HasAllSymbols() const noexcept;

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

    protected:
        [[nodiscard]] CollinsGisinEntry make_element_no_checks(Tensor::IndexView index) const override;

        [[nodiscard]] std::string get_name() const override {
            return "Collins-Gisin tensor";
        }

    public:
        friend class CollinsGisinEntry;
        friend class CollinsGisinIterator;
        friend class ProbabilityTensor;

    };

    class CollinsGisinIterator : public CollinsGisin::Iterator {
    public:
        /**
         * Construct iterator over range.
         */
        CollinsGisinIterator(const CollinsGisin& cg, CollinsGisinIndex&& first, CollinsGisinIndex&& last)
            : CollinsGisin::Iterator{cg, std::move(first), std::move(last)} { }

        /**
         * 'End' iterator constructor.
         */
        explicit CollinsGisinIterator(const CollinsGisin& cg) : CollinsGisin::Iterator{cg} { }

        /**
          * Pointed to operator sequence.
          */
        [[nodiscard]] inline const OperatorSequence& sequence() const {
            return this->operator*().sequence;
        }

        /**
         * Pointed to symbol ID, if known.
         */
        [[nodiscard]] inline  symbol_name_t symbol_id() const {
            return this->operator*().symbol_id;
        }

        /**
         * Pointed to real basis element, if known.
         */
        [[nodiscard]] inline ptrdiff_t real_basis() const {
            return this->operator*().real_index;
        }
    };

    class CollinsGisinRange {
    private:
        const CollinsGisin& collinsGisin;
        const CollinsGisinIndex first;
        const CollinsGisinIndex last;

        const CollinsGisinIterator iter_end;

    public:
        CollinsGisinRange(const CollinsGisin& cg, CollinsGisinIndex&& first, CollinsGisinIndex&& last)
                : collinsGisin{cg}, first(std::move(first)), last(std::move(last)),
                  iter_end(cg) {
        }

        [[nodiscard]] CollinsGisinIterator begin() const {
            return CollinsGisinIterator{collinsGisin, CollinsGisinIndex(this->first), CollinsGisinIndex(this->last)};
        }

        [[nodiscard]] inline const CollinsGisinIterator& end() const noexcept {
            return this->iter_end;
        }
    };

}