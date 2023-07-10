/**
 * make_explicit.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "mex_function.h"

#include "import/read_measurement_indices.h"

#include <vector>

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions {

    struct MakeExplicitParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The requested measurements  */
        std::vector<RawIndexPair> free_indices;

        /** The fixed outcomes */
        std::vector<RawIndexTriplet> fixed_indices;

        /** The supplied values (flattened) */
        std::vector<double> values;

        /** The type of output */
        enum class OutputType {
            SymbolCell,
            Polynomial
        } output_type = OutputType::SymbolCell;

        /** Export conditional measurement ? */
        bool is_conditional = false;

        /** True if a conditional probability is required and requested. */
        [[nodiscard]] inline bool conditional() const noexcept {
            return this->is_conditional && !this->fixed_indices.empty();
        }

    public:
        explicit MakeExplicitParams(SortedInputs&& structuredInputs);

    };

    class MakeExplicit : public ParameterizedMexFunction<MakeExplicitParams, MEXEntryPointID::MakeExplicit> {
    public:
        explicit MakeExplicit(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MakeExplicitParams &input) override;

        void extra_input_checks(MakeExplicitParams &input) const override;

    private:
        void check_count(const MatrixSystem &system, size_t slice_size, MakeExplicitParams &input);

    };

}