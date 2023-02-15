/**
 * substitution_list.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substitution_list.h"
#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "utilities/dynamic_bitset.h"

#include <iostream>

namespace Moment {

    SubstitutionList::SubstitutionList(SubstitutionList::raw_map_t map) noexcept
        : raw_sub_data(std::move(map)) {
        // Populate computed values
        for (auto [symbol_id, value] : this->raw_sub_data) {
            this->sub_data.emplace_hint(this->sub_data.end(),
                                        std::make_pair(symbol_id, SymbolExpression{value != 0 ? 1 : 0, value, false}));

        }
    }


    void SubstitutionList::infer_substitutions(const MatrixSystem& system) {

        // For now, only do extra inference for inflation matrix systems
        const auto * asInfMS = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
        if (nullptr == asInfMS ) {
            return;
        }
        const auto& inflation_system = *asInfMS;

        // Go through factorized symbols...
        const auto& factors = inflation_system.Factors();
        for (const auto& factor : factors) {
            // Skip if not factorized (basic substitutions should be handled from raw map!)
            if (factor.fundamental()) {
                continue;
            }

            // Look up raw data...
            DynamicBitset<uint64_t> matched{factor.canonical.symbols.size()};

            size_t check_index = 0;
            double new_weight = 1.0;
            for (auto symbol : factor.canonical.symbols) {
                auto raw_sub_iter = raw_sub_data.find(symbol);
                if (raw_sub_iter != raw_sub_data.end()) {
                    matched.set(check_index);
                    new_weight *= raw_sub_iter->second;
                }
                ++check_index;
            }

            // No matches, leave alone
            if (matched.empty()) {
                continue;
            }

            // All matches, replace by scalar
            if (matched.count() == factor.canonical.symbols.size()) {
                this->sub_data.insert(std::make_pair(factor.id,
                                                     SymbolExpression{new_weight != 0.0 ? 1 : 0, new_weight}));
                continue;
            }

            // Weight is zero, replace by scalar zero
            if (new_weight == 0) {
                this->sub_data.insert(std::make_pair(factor.id, SymbolExpression{0, 0.0}));
                continue;
            }

            // Otherwise, we create a new factor sequence...
            std::vector<symbol_name_t> new_factor_sequence;
            new_factor_sequence.reserve(factor.canonical.symbols.size() - matched.count());
            for (size_t rewrite_index = 0, index_max = factor.canonical.symbols.size();
                 rewrite_index < index_max; ++rewrite_index) {
                if (!matched.test(rewrite_index)) {
                    new_factor_sequence.push_back(factor.canonical.symbols[rewrite_index]);
                }
            }

            // Look up remaining unfactorized components
            auto maybe_index = factors.find_index_by_factors(new_factor_sequence);
            if (!maybe_index.has_value()) {
                throw std::logic_error{"Could not find symbol associated with partially substituted factor."};
            }
            symbol_name_t new_index = maybe_index.value();

            // Add a substitution rule for this
            this->sub_data.insert(std::make_pair(factor.id, SymbolExpression{new_index, new_weight}));
        }
    }

    SymbolExpression SubstitutionList::substitute(const SymbolExpression& i) const {
        // Look up in table
        auto found_sub_iter = this->sub_data.find(i.id);

        // If nothing found, pass symbol expression through.
        if (found_sub_iter == this->sub_data.end()) {
            return i;
        }

        const auto& sub_symbol = found_sub_iter->second;


        // Otherwise, apply substitution
        const bool conjugated = (sub_symbol.id != 0) && (sub_symbol.conjugated != i.conjugated);
        return SymbolExpression{sub_symbol.id, i.factor * sub_symbol.factor, conjugated};
    }


    std::unique_ptr<SquareMatrix<SymbolExpression>>
    SubstitutionList::operator()(const SquareMatrix<SymbolExpression> &source_matrix) const {
        std::vector<SymbolExpression> new_matrix_data;
        new_matrix_data.reserve(source_matrix.dimension*source_matrix.dimension);
        for (const auto& entry : source_matrix) {
            new_matrix_data.emplace_back(this->substitute(entry));
        }
        return std::make_unique<SquareMatrix<SymbolExpression>>(source_matrix.dimension, std::move(new_matrix_data));
    }

    std::ostream& operator<<(std::ostream& os, const SubstitutionList& list) {
        return list.write_list(os, "\n");
    }

    std::ostream &SubstitutionList::write_list(std::ostream &os, const std::string& delimiter) const {
        bool done_one = false;
        for (auto [key, expr] : this->sub_data) {
            if (done_one) {
                os << delimiter;
            } else {
                done_one = true;
            }
            os << key << " -> " << expr;
        }
        return os;
    }

}