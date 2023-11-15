/**
 * plus.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../../mtk_function.h"
#include "integer_types.h"

#include "import/algebraic_operand.h"

#include <span>
#include <string>

namespace Moment::mex::functions  {

    struct PlusParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        AlgebraicOperand lhs;
        AlgebraicOperand rhs;

//        enum class DistributionMode {
//            /** Broadcast LHS to many RHS. */
//            OneToMany,
//            /** Many LHS to broadcast RHS. */
//            ManyToOne,
//            /** Element-wise add. (Incorporates OneToOne). */
//            ManyToMany
//        } distribution_mode;

        std::vector<size_t> output_shape;
        size_t output_size;

        enum class OutputMode {
            Index,
            SymbolCell,
            SequencesWithSymbolInfo,
            String
        } output_mode = OutputMode::SymbolCell;

    public:
        explicit PlusParams(SortedInputs&& inputs);
    };

    class Plus : public ParameterizedMTKFunction<PlusParams, MTKEntryPointID::Plus> {
    public:
        explicit Plus(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, PlusParams &input) override;

        void extra_input_checks(PlusParams &input) const override;
    };
}
