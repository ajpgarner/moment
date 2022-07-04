/**
 * collins_gisin.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol.h"
#include "operator_sequence.h"
#include "party.h"
#include "utilities/recursive_index.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <span>
#include <vector>

namespace NPATK {

    namespace errors {
        class cg_form_error : std::logic_error {
        public:
            explicit cg_form_error(const std::string& what) : std::logic_error(what) { }
        };
    }

    class MomentMatrix;

    /** Returns a list of explicit symbols, according to the parties and measurements chosen. */
    class CollinsGisinForm {
    public:
        using storage_t = std::vector<symbol_name_t>;

    public:
        const size_t Level;
        const std::vector<size_t> OperatorCounts;

    private:
        storage_t data;
        RecursiveDoubleIndex indices;

    public:
        CollinsGisinForm(const MomentMatrix& mm, size_t level);

        /**
         * Gets a span of *all* symbols corresponding to the supplied measurement indices.
         * @param mmtIndices A sorted list of global indices of the measurement.
         */
        [[nodiscard]] std::span<const symbol_name_t> get(std::span<const size_t> mmtIndices) const;

        [[nodiscard]] inline std::span<const symbol_name_t> get(std::initializer_list<size_t> mmtIndices) const {
            std::vector<size_t> v{mmtIndices};
            return get(v);
        }

        /**
         * Gets a filtered list of symbols corresponding to the supplied measurement indices, fixing some of the
         * measurement outcomes.
         * @param mmtIndices A sorted list of global indices of the measurement.
         * @param fixedIndices List of outcome indices, or -1 if not fixed.
         * @return
         */
        [[nodiscard]] std::vector<symbol_name_t> get(std::span<const size_t> mmtIndices,
                                                     std::span<const oper_name_t> fixedOutcomes) const;

        [[nodiscard]] inline std::vector<symbol_name_t> get(std::initializer_list<size_t> mmtIndices,
                                                  std::initializer_list<oper_name_t> fixedOutcomes) const {
            std::vector<size_t> i{mmtIndices};
            std::vector<oper_name_t> o{fixedOutcomes};
            return get(i, o);
        }


    };
}