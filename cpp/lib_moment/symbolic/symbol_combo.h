/**
 * symbol_combo.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "symbol_expression.h"
#include "utilities/small_vector.h"

#include <algorithm>
#include <map>
#include <iosfwd>
#include <utility>
#include <vector>

namespace Moment {
    class SymbolCombo {
    public:
        /**
         * Storage for linear  combination of symbolic expressions.
         * Monomial on stack, polynomial on heap.
         * */
        using storage_t = SmallVector<SymbolExpression, 1>;

    private:
        storage_t data;

    public:

        SymbolCombo() = default;

        SymbolCombo(const SymbolCombo& rhs) = default;

        SymbolCombo(SymbolCombo&& rhs) = default;

        explicit SymbolCombo(storage_t input);

        explicit SymbolCombo(const std::map<symbol_name_t, double>& input);

        SymbolCombo(std::initializer_list<SymbolExpression> input)
            : SymbolCombo{storage_t{input}} { }

        [[nodiscard]] size_t size() const noexcept { return this->data.size(); }
        [[nodiscard]] bool empty() const noexcept { return this->data.empty(); }
        [[nodiscard]] auto begin() const noexcept { return this->data.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->data.cend(); }
        [[nodiscard]] const SymbolExpression& operator[](size_t index) const noexcept { return this->data[index]; }

        SymbolCombo& operator+=(const SymbolCombo& rhs);

        [[nodiscard]] friend SymbolCombo operator+(const SymbolCombo& lhs, const SymbolCombo& rhs) {
            SymbolCombo output{lhs};
            output += rhs;
            return output;
        }

        SymbolCombo& operator*=(double factor) noexcept;

        [[nodiscard]] friend SymbolCombo operator*(SymbolCombo lhs, const double factor) noexcept {
            lhs *= factor;
            return lhs;
        }

        bool operator==(const SymbolCombo& rhs) const noexcept;

        inline bool operator!=(const SymbolCombo& rhs) const noexcept {
            return !(this->operator==(rhs));
        }

        friend std::ostream& operator<<(std::ostream& os, const SymbolCombo& combo);

    private:
        void order();


    };

}