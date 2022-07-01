/**
 * implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "moment_matrix.h"
#include "symbolic/linear_combo.h"
#include "utilities/recursive_index.h"

#include <vector>
#include <stdexcept>

namespace NPATK {

    class MultiMmtIterator;

    namespace errors {
        class bad_implicit_symbol : std::logic_error {
        public:
            PMOIndex index;

            bad_implicit_symbol(PMOIndex index, const std::string& what) : index{index}, std::logic_error(what) { }
        };
    }

    /**
     * Calculate the 'missing' marginals/probabilities from the Gisin form.
     */
    class ImplicitSymbols {
    public:
        using SymbolCombo = LinearCombo<symbol_name_t, double>;

        struct PMODefinition {
            symbol_name_t symbol_id = 0;
            SymbolCombo expression{};

        public:
            constexpr PMODefinition(symbol_name_t symbol_id,
                                    SymbolCombo expr)
                : symbol_id{symbol_id}, expression(std::move(expr)) { }
        };

        class ProbabilityTable {
        public:
            using storage_t = std::vector<PMODefinition>;
            using const_iterator_t = storage_t::const_iterator;
            const size_t MaxLevel;

        private:
            storage_t tableData{};
            RecursiveDoubleIndex indices;

        public:
            constexpr ProbabilityTable(size_t width, size_t max_depth) :
                    MaxLevel{max_depth}, indices{width, max_depth} {
            }

            //constexpr ProbabilityTable(size_t level, storage_t data, RecursiveDoubleIndex indexData) noexcept
            //    : MaxLevel{level}, tableData{std::move(data)}, indices{std::move(indexData)} { }

            [[nodiscard]] constexpr const std::vector<PMODefinition>& Data() const noexcept {
                return this->tableData;
            }

            [[nodiscard]] constexpr const RecursiveDoubleIndex& Indices() const noexcept {
                return this->indices;
            }

            [[nodiscard]] auto get(const std::span<size_t> mmtIndex) const noexcept {
                auto [first, last] = this->indices.access(mmtIndex);
                if ((first < 0) || (first >= last)) {
                    return std::span<const PMODefinition>(tableData.begin(), 0);
                }
                assert(last <= tableData.size());
                return std::span<const PMODefinition>(tableData.begin() + first, last - first);
            }

            friend class ImplicitSymbols;
        };

    private:
        const MomentMatrix& momentMatrix;
        const CollinsGisinForm& cgForm;
        const Context& context;

    public:
        const size_t MaxSequenceLength;

    private:
        std::unique_ptr<ProbabilityTable> probabilityTable;

    public:
        explicit ImplicitSymbols(const MomentMatrix& mm);

        [[nodiscard]] const ProbabilityTable& Table() const noexcept { return *this->probabilityTable; }

    private:
        size_t generateLevelZero(size_t& index_cursor);
        size_t generateLevelOne(size_t& index_cursor);
        size_t generateMoreLevels(size_t level, size_t& index_cursor);
        size_t generateFromCurrentStack(const MultiMmtIterator& stack, size_t& index_cursor,
                                        std::vector<PMODefinition>& entries, RecursiveDoubleIndex& indices);

    };

}