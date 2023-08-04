/**
 * factor_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "factor_table.h"

#include "../contextual_os_helper.h"
#include "inflation_context.h"

#include "matrix/monomial_matrix.h"

#include "symbolic/symbol_table.h"
#include "symbolic/polynomial_factory.h"

#include <algorithm>
#include <sstream>



namespace Moment::Inflation {
    namespace errors {
        namespace {
            std::string make_us_err_msg(const std::string& bad_str) {
                std::stringstream errSS;
                errSS << "No symbol found in table for factored expression \"" << bad_str << "\"";
                return errSS.str();
            }
        }

        unknown_symbol::unknown_symbol(const std::string &bad_str)
                : range_error(make_us_err_msg(bad_str)), unknown{bad_str} {

        }
    }

    std::string FactorTable::FactorEntry::sequence_string() const {

        std::stringstream ss;

        if (this->canonical.sequences.size() == 1) {
            if (this->canonical.sequences[0].empty()) {
                if (this->canonical.sequences[0].zero()) {
                    return "0";
                } else {
                    return "1";
                }
            }
        }

        for (const auto& seq : this->canonical.sequences) {
            ss << "<" << seq << ">";
        }
        return ss.str();
    }


    FactorTable::FactorTable(const InflationContext& context, SymbolTable& symbols_in)
        : context{context}, symbols{symbols_in} {
        this->on_new_symbols_added();
    }

    size_t FactorTable::on_new_symbols_added() {
        // Do nothing if symbol table is up-to-date.
        if (this->entries.size() == this->symbols.size()) {
            return 0;
        }

        return this->check_for_new_factors();
    }

    size_t FactorTable::check_for_new_factors() {
        const auto next_id = static_cast<symbol_name_t>(this->entries.size());
        const auto up_to_id = static_cast<symbol_name_t>(this->symbols.size());

        // Early exit, if no new symbols
        if (next_id == up_to_id) {
            return 0;
        }

        // Check for factors of new symbols
        for (symbol_name_t symbol_index = next_id; symbol_index < up_to_id; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back(symbol.Id());
            auto& entry = this->entries.back();

            // Check for factors
            entry.raw.sequences = this->context.factorize(symbol.sequence());

            // Check canonical form of factors
            entry.canonical.sequences.reserve(entry.raw.sequences.size());
            entry.canonical.symbols.reserve(entry.raw.sequences.size());

            // Get symbols for each canonical sequence
            for (const auto& factor_raw_seq : entry.raw.sequences) {
                entry.canonical.sequences.emplace_back(this->context.canonical_moment(factor_raw_seq));
                const auto& factor_seq = entry.canonical.sequences.back();

                // Try to find ID of (canonical) factor
                auto where = this->symbols.where(factor_seq);
                if (where != nullptr) {
                    assert(where->is_hermitian());
                    entry.canonical.symbols.emplace_back(where->Id());
                } else {
                    Symbol us{factor_seq};
                    auto new_entry = this->symbols.merge_in(std::move(us));
                    entry.canonical.symbols.emplace_back(new_entry);
                }
            }

            // Canonical symbols should be sorted in factor entry
            std::sort(entry.canonical.symbols.begin(), entry.canonical.symbols.end());
            // Add to index tree
            this->index_tree.add(entry.canonical.symbols, entry.id);
        }

        // Newly added symbols automatically should not factorize, and will be canonical
        const auto extra_symbols = static_cast<symbol_name_t>(this->symbols.size());
        for (symbol_name_t symbol_index = up_to_id; symbol_index < extra_symbols; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back(symbol.Id());
            auto& entry = this->entries.back();
            entry.id = symbol.Id();
            entry.raw.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.canonical.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.canonical.symbols = std::vector<symbol_name_t>{entry.id};
            this->index_tree.add(entry.canonical.symbols, entry.id);
        }

        // Count factors
        for (size_t entry_index = next_id; entry_index < extra_symbols; ++entry_index) {
            const auto& factor_entry = this->entries[entry_index];
            // Non-trivial factors?
            if (factor_entry.canonical.symbols.size() > 1) {
                for (const auto& factor_symbol : factor_entry.canonical.symbols) {
                    assert(factor_symbol < this->entries.size());
                    this->entries[factor_symbol].appearances += 1;
                }
            }
        }

        return extra_symbols - next_id;
    }

    void FactorTable::register_new(symbol_name_t id, std::vector<symbol_name_t> factors) {
        this->entries.emplace_back(id);
        assert(this->entries.size() == id+1);
        auto& new_entry = this->entries.back();
        new_entry.canonical.symbols.swap(factors);

        // Look up associated sequences
        new_entry.canonical.sequences.reserve(new_entry.canonical.symbols.size());
        for (auto sym_id : new_entry.canonical.symbols) {
            new_entry.canonical.sequences.push_back(symbols[sym_id].sequence());
        }

        // Create index
        index_tree.add(new_entry.canonical.symbols, new_entry.id);

    }

    symbol_name_t FactorTable::try_multiply(symbol_name_t lhs, symbol_name_t rhs) const {
        // Do not multiply nonsense symbols
        assert((lhs >= 0) && (lhs < this->symbols.size()));
        assert((rhs >= 0) && (rhs < this->symbols.size()));

        // Anything times 0 is 0
        if ((lhs == 0) || (rhs == 0)) {
            return 0;
        }
        // Anything times 1 is itself
        if (lhs == 1) {
            return rhs;
        }
        // Anything times 1 is itself
        if (rhs == 1) {
            return lhs;
        }

        // Is either side non-fundamental?
        if (!this->entries[lhs].fundamental() || !this->entries[rhs].fundamental()) {;
            return this->try_multiply(std::vector<symbol_name_t>{lhs, rhs});
        }

        // Remaining possibilities are non-trivial: see if we can find matching factor.
        std::array<symbol_name_t, 2> idx{lhs < rhs ? lhs : rhs, lhs < rhs ? rhs : lhs};
        auto factor_entry = this->find_index_by_factors(idx);
        if (!factor_entry.has_value() || (factor_entry.value() >= this->entries.size())) {
            StringFormatContext sfc{this->context, this->symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            throw errors::unknown_symbol{
                make_contextualized_string(sfc, [this, lhs, rhs](ContextualOS& os) {
                    os << this->symbols[lhs].ForwardDisplayElement() << this->symbols[rhs].ForwardDisplayElement();
                })};
        }
        return this->entries[factor_entry.value()].id;
    }


    symbol_name_t FactorTable::try_multiply(std::vector<symbol_name_t> multiplicands) const {
        // If zero or one entries, early exit
        if (multiplicands.empty()) {
            return 0;
        }
        if (1 == multiplicands.size()) {
            return multiplicands[0];
        }

        // If a zero is in list, factor is zero.
        if (std::any_of(multiplicands.begin(), multiplicands.end(), [](auto x){ return x == 0;})) {
            return 0;
        }

        // Remove any 1s
        auto erase_iter = std::remove(multiplicands.begin(), multiplicands.end(), 1);
        if (erase_iter != multiplicands.end()) {
            multiplicands.erase(erase_iter, multiplicands.end());

            // If zero or one entries, early exit again
            if (multiplicands.empty()) {
                return 1;
            }
            if (multiplicands.size() == 1) {
                return multiplicands[0];
            }
        }

        // Any non-fundamental variables?
        const bool any_non_fundamental = std::any_of(multiplicands.begin(), multiplicands.end(),
            [this](auto x) {
                assert(x < this->entries.size());
                return !this->entries[x].fundamental();
        });

        // Make fundamental, if necessary
        if (any_non_fundamental) {
            std::vector<symbol_name_t> fundamental;
            for (auto symbol_id: multiplicands) {
                assert (symbol_id < this->symbols.size());
                const auto &factor_entry = this->entries[symbol_id];
                if (factor_entry.fundamental()) {
                    fundamental.emplace_back(symbol_id);
                } else {
                    std::copy(factor_entry.canonical.symbols.begin(), factor_entry.canonical.symbols.end(),
                              std::back_inserter(fundamental));
                }
            }
            std::swap(multiplicands, fundamental);
        }

        // Sort remainder
        std::sort(multiplicands.begin(), multiplicands.end());

        // Query as canonical container
        return try_multiply_canonical(multiplicands);

    }

    symbol_name_t FactorTable::try_multiply_canonical(std::span<const symbol_name_t> multiplicands) const {

        assert(std::all_of(multiplicands.begin(), multiplicands.end(), [this](auto x) {
            return ((x >= 0)) && (x < this->symbols.size());
        }));

        // General case:
        auto factor_entry = this->find_index_by_factors(multiplicands);
        if (!factor_entry.has_value() || (factor_entry.value() >= this->entries.size())) {
            StringFormatContext sfc{this->context, this->symbols};
            sfc.format_info.show_braces = true;
            sfc.format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;

            throw errors::unknown_symbol{
                make_contextualized_string(sfc, [this, &multiplicands](ContextualOS& os) {
                    for (auto sym: multiplicands) {
                        os << this->symbols[sym].ForwardDisplayElement();
                    }
                })};
        }
        return this->entries[factor_entry.value()].id;
    }

    Polynomial FactorTable::try_multiply(const PolynomialFactory &factory,
                                         const Polynomial& lhs, const Polynomial &rhs) const {
        // Multiply by zero is zero.
        if (rhs.empty()) {
            return Polynomial{};
        }

        // Monomial?
        if (rhs.is_monomial()) {
            const auto& rhs_mono = rhs.back();
            // Scalar?
            if (rhs_mono.id == 1) {
                return lhs * rhs_mono.factor;
            }
        }

        // General multiplication:
        Polynomial::storage_t output;
        output.reserve(lhs.size() * rhs.size());

        for (const auto& lhs_expr : lhs) {
            for (const auto& rhs_expr : rhs) {
                // For now, throw on CC.
                if ((lhs_expr.conjugated || rhs_expr.conjugated)) {
                    std::stringstream badSS;
                    badSS << lhs_expr << " * " << rhs_expr;
                    throw errors::unknown_symbol{badSS.str()};
                }
                auto combined_symbol_id = this->try_multiply(lhs_expr.id, rhs_expr.id);
                auto combined_factor = lhs_expr.factor * rhs_expr.factor;
                output.emplace_back(combined_symbol_id, combined_factor, false);
            }
        }

        return factory(std::move(output));
    }



    void FactorTable::combine_symbolic_factors(std::vector<symbol_name_t>& output,
                                          const std::vector<symbol_name_t>& left,
                                          const std::vector<symbol_name_t>& right) {
        // Ensure output is empty befor merge begins
        output.clear();

        // First, no factors on either side -> identity.
        if (left.empty() && right.empty()) {
            output.emplace_back(1);
            return;
        }

        assert(std::is_sorted(left.cbegin(), left.cend()));
        assert(std::is_sorted(right.cbegin(), right.cend()));

        // Copy and sort factors

        output.reserve(left.size() + right.size());
        std::merge(left.cbegin(), left.cend(), right.cbegin(), right.cend(), std::back_inserter(output));

        // If "0" is somehow a factor of either left or right, the product is zero
        assert(!output.empty());
        if (output[0] == 0) [[unlikely]] {
            output.emplace_back(0);
            return;
        }

        // Now, if we have more than one factor, prune identities (unless only identity)
        if (output.size() > 1) {
            auto first_non_id = std::upper_bound(output.begin(), output.end(), 1);
            output.erase(output.begin(), first_non_id);
            // Factors were  1 x 1 x ... x 1
            if (output.empty()) {
                output.emplace_back(1);
                return;
            }
        }
    }




}