/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"
#include "../operator_sequence.h"

#include "operator_matrix.h"

#include <cassert>
#include <map>
#include <memory>
#include <span>

namespace NPATK {

    class MomentMatrix : public OperatorMatrix {
    public:

        /** The Level of moment matrix defined */
        const size_t hierarchy_level;

    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param symbols Source of existing symbols, sink for any new symbols first appearing in the matrix.
         * @param level The hierarchy depth.
         */
        MomentMatrix(const Context& context, SymbolTable& symbols, size_t level);

        MomentMatrix(const MomentMatrix&) = delete;

        MomentMatrix(MomentMatrix&& src) noexcept;

        /** Destructor */
        ~MomentMatrix() override;

        /**
         * The hierarchy depth of this moment matrix.
         */
        [[nodiscard]] constexpr size_t Level() const noexcept { return this->hierarchy_level; }

    };


}