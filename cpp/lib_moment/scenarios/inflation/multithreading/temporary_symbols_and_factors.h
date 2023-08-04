/**
 * temporary_symbols_and_factors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include "utilities/index_tree.h"
#include "utilities/maintains_mutex.h"

#include <vector>

namespace Moment {
    namespace Inflation {
        class InflationContext;

        class FactorTable;
    }
    class OperatorSequence;
    class SymbolTable;

    namespace Multithreading {

        class TemporarySymbolsAndFactors : private MaintainsMutex {
            SymbolTable& symbols;
            Inflation::FactorTable& factors;

            const symbol_name_t first_symbol_id;
            symbol_name_t next_symbol_id;

            std::vector<std::unique_ptr<std::vector<symbol_name_t>>> new_factors;
            std::vector<std::unique_ptr<std::vector<OperatorSequence>>> new_op_seqs;
            IndexTree<symbol_name_t, symbol_name_t> index_tree;

        public:
            TemporarySymbolsAndFactors(SymbolTable& symbols, Inflation::FactorTable& factors);
            ~TemporarySymbolsAndFactors() noexcept;

            const std::vector<symbol_name_t>& find_factors_by_symbol_id(const symbol_name_t symbol_id);

            symbol_name_t find_or_register_factors(std::span<const symbol_name_t> joint_factors);

            void register_new_symbols_and_factors();

            /**
             * Gets the number of additional factors registered in the temporary bank.
             * Function contends for mutex, but is provided for debug/test purposes.
             */
            [[nodiscard]] inline symbol_name_t additional_symbol_count() const noexcept {
                auto read_lock = this->get_read_lock();
                return this->next_symbol_id - this->first_symbol_id;
            }

        };
    }
}