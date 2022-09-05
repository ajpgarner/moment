/**
 * matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"
#include "symbol_table.h"
#include "moment_matrix.h"
#include "localizing_matrix.h"

#include <memory>
#include <vector>

namespace NPATK {

    class MatrixSystem {
    private:
        std::shared_ptr<Context> context;
        SymbolTable symbol_table;

        std::vector<std::unique_ptr<MomentMatrix>> momentMatrices;
        std::vector<std::unique_ptr<LocalizingMatrix>> localizingMatrices;

    public:
        explicit MatrixSystem(std::shared_ptr<Context> context);

        /**
         * Check if a MomentMatrix has been generated for a particular hierarchy level.
         * @param level The hierarchy depth.
         * @return True, if MomentMatrix exists for particular level.
         */
        [[nodiscard]] bool HasLevel(size_t level) const noexcept {
            if (level >= momentMatrices.size()) {
                return false;
            }
            return static_cast<bool>(momentMatrices[level]);
        }

        /**
         * Returns the MomentMatrix for a particular hierarchy level.
         * @param level The hierarchy depth.
         * @return True, if MomentMatrix exists for particular level.
         */
        [[nodiscard]] const class MomentMatrix& MomentMatrix(size_t level) const;

        class MomentMatrix& CreateMomentMatrix(size_t level);



    };
}