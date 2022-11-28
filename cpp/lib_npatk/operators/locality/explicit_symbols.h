/**
 * explicit_symbol.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "joint_measurement_index.h"
#include "party.h"

#include "operators/operator_sequence.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include "integer_types.h"

#include <initializer_list>
#include <stdexcept>
#include <span>
#include <vector>

namespace NPATK {

    namespace errors {
        class cg_form_error : std::logic_error {
        public:
            explicit cg_form_error(const std::string& what) : std::logic_error(what) { }
        };
    }

    class LocalityMatrixSystem;
    class InflationMatrixSystem;

    /** Data record for CGI */
    struct ExplicitSymbolEntry {
        symbol_name_t symbol_id;
        ptrdiff_t real_basis;
    };

    /** An index of explicit real operators, according to the parties and measurements chosen. */
    class ExplicitSymbolIndex {
    public:
        using storage_t = std::vector<ExplicitSymbolEntry>;

    public:
        /** The maximum number of operators in a sequence */
        const size_t Level;


        /** The number of operators from each party */
        const std::vector<size_t> OperatorCounts;

    private:
        storage_t data;
        JointMeasurementIndex indices;

    public:
        /**
         * Construct explicit symbol table for locality system
         */
        ExplicitSymbolIndex(const LocalityMatrixSystem& ms, size_t level);

        /**
         * Construct explicit symbol table for inflation system
         */
        ExplicitSymbolIndex(const InflationMatrixSystem& ms, size_t level);

        /**
         * Gets a span of *all* symbols corresponding to the supplied measurement indices.
         * @param mmtIndices A sorted list of global indices of the measurement.
         */
        [[nodiscard]] std::span<const ExplicitSymbolEntry> get(std::span<const size_t> mmtIndices) const;

        /**
         * Gets a span of *all* symbols corresponding to the supplied measurement indices.
         * @param mmtIndices A sorted list of global indices of the measurement.
         */
        [[nodiscard]] inline std::span<const ExplicitSymbolEntry> get(std::initializer_list<size_t> mmtIndices) const {
            std::vector<size_t> v{mmtIndices};
            return get(v);
        }

        /**
         * Gets a filtered list of symbols corresponding to the supplied measurement indices, fixing some of the
         * measurement outcomes.
         * @param mmtIndices A sorted list of global indices of the measurement.
         * @param fixedOutcomes List of outcome indices, or -1 if not fixed.
         * @return
         */
        [[nodiscard]] std::vector<ExplicitSymbolEntry> get(std::span<const size_t> mmtIndices,
                                                           std::span<const oper_name_t> fixedOutcomes) const;

        /**
        * Gets a filtered list of symbols corresponding to the supplied measurement indices, fixing some of the
        * measurement outcomes.
        * @param mmtIndices A sorted list of global indices of the measurement.
        * @param fixedOutcomes List of outcome indices, or -1 if not fixed.
        * @return
        */
        [[nodiscard]] inline std::vector<ExplicitSymbolEntry> get(std::initializer_list<size_t> mmtIndices,
                                                                  std::initializer_list<oper_name_t> fixedOutcomes) const {
            std::vector<size_t> i{mmtIndices};
            std::vector<oper_name_t> o{fixedOutcomes};
            return get(i, o);
        }


    };
}