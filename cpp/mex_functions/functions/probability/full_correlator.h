/**
 * full_correlator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mtk_function.h"

#include "import/matrix_system_id.h"
#include "import/read_measurement_indices.h"

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions  {

    struct FullCorrelatorParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        enum class ExportShape {
            WholeTensor,
            OneCorrelator
        } export_shape = ExportShape::WholeTensor;

        /** How do we want to export */
        enum struct OutputMode {
            /** Export specification of polynomials. */
            OperatorSequences,
            /** Export specification of polynomials, but also include symbol info. */
            OperatorSequencesWithSymbolInfo,
            /** Export as cell array of symbols. */
            Symbols,
            /** Export as strings */
            Strings
        } output_mode = OutputMode::Symbols;


    public:
        explicit FullCorrelatorParams(SortedInputs&& inputs);

        std::vector<RawIndexPair> measurementIndices;
    };

    class FullCorrelator : public ParameterizedMTKFunction<FullCorrelatorParams, MTKEntryPointID::FullCorrelator> {
    public:
        explicit FullCorrelator(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, FullCorrelatorParams &input) override;

        void export_whole_tensor(IOArgumentRange output, FullCorrelatorParams &input, MatrixSystem& system);

        void export_one_correlator(IOArgumentRange output, FullCorrelatorParams &input, MatrixSystem& system);

    };

}
