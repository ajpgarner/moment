/**
 * substitution_list.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "symbol_expression.h"

#include <iosfwd>
#include <map>
#include <span>

namespace Moment {
    class MatrixSystem;

    /**
     * Perform substitutions at the symbol level, reducing symbols with numbers.
     */
    class SubstitutionList {
    public:
        using raw_map_t = std::map<symbol_name_t, double>;
        using computed_map_t = std::map<symbol_name_t, SymbolExpression>;
        using pair_t = raw_map_t::value_type;

    private:
        raw_map_t raw_sub_data;
        computed_map_t sub_data;

    public:
        /**
         * Construct a list of numerical substitutions of symbols
         * @param map A map, from symbol identities to real numeric values.
         */
        explicit SubstitutionList(raw_map_t map) noexcept;

        /**
         * Construct a list of numerical substitutions of symbols
         * @param flat_data A list of pairs, from symbol identities to real numeric values.
         */
        inline explicit SubstitutionList(std::span<const pair_t>& flat_data)
            : SubstitutionList{raw_map_t{flat_data.begin(), flat_data.end()}} { }

        /**
         * Use a matrix system to infer any additional symbolic substitutions required.
         * @param system The matrix system.
         */
        void infer_substitutions(const MatrixSystem& system);

        /**
         * Replace symbol expression with substitution.
         */
        [[nodiscard]] SymbolExpression substitute(const SymbolExpression& i) const;

        /**
         * Replace symbol expression with substitution.
         */
        [[nodiscard]] inline auto operator()(const SymbolExpression& i) const {
            return this->substitute(i);
        }

        friend std::ostream& operator<<(std::ostream& os, const SubstitutionList& list);

    };
}