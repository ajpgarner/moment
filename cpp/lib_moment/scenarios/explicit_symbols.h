/**
 * explicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include "joint_measurement_index.h"

#include <initializer_list>
#include <stdexcept>
#include <span>
#include <vector>

namespace Moment {

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

    protected:
        storage_t data;

        /** Operators per measurement */
        std::vector<size_t> operator_counts;

    public:
        virtual ~ExplicitSymbolIndex() = default;

        /**
         * Gets a span of *all* symbols corresponding to the supplied measurement indices.
         * @param mmtIndices A sorted list of global indices of the measurement.
         */
        [[nodiscard]] virtual std::span<const ExplicitSymbolEntry> get(std::span<const size_t> mmtIndices) const = 0;

        /**
         * Gets a filtered list of symbols corresponding to the supplied measurement indices, fixing some of the
         * measurement outcomes.
         * @param mmtIndices A sorted list of global indices of the measurement.
         * @param fixedOutcomes List of outcome indices, or -1 if not fixed.
         * @return
         */
        [[nodiscard]] std::vector<ExplicitSymbolEntry>
        get(std::span<const size_t> mmtIndices, std::span<const oper_name_t> fixedOutcomes) const;

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
        [[nodiscard]] inline std::vector<ExplicitSymbolEntry> get(std::initializer_list<size_t> mmtIndices,
                                                                  std::initializer_list<oper_name_t> fixedOutcomes) const {
            std::vector<size_t> i{mmtIndices};
            std::vector<oper_name_t> o{fixedOutcomes};
            return get(i, o);
        }

    protected:
        /**
         * Construct explicit symbol table for locality system
         */
        explicit ExplicitSymbolIndex(size_t level, std::vector<size_t> op_counts)
            : Level{level}, operator_counts(std::move(op_counts)) { }
    };

}