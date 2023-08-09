/**
 * collins_gisin.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"

#include "import/read_measurement_indices.h"

namespace Moment {
    class MatrixSystem;
}


namespace Moment::mex::errors {
    constexpr char missing_cg[] = "missing_cg";
}

namespace Moment::mex::functions  {

    struct CollinsGisinParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        enum class ExportShape {
            WholeTensor,
            OneMeasurement,
            OneOutcome
        } export_shape = ExportShape::WholeTensor;

        enum class OutputType {
            Sequences,
            SequencesWithSymbolInfo,
            SymbolIds,
            SequenceStrings
        } output_type = OutputType::Sequences;

    public:
        explicit CollinsGisinParams(SortedInputs&& inputs);

        std::vector<RawIndexPair> freeMeasurements;
        std::vector<RawIndexTriplet> fixedOutcomes;
    };

    class CollinsGisin : public ParameterizedMTKFunction<CollinsGisinParams, MTKEntryPointID::CollinsGisin> {
    public:
        explicit CollinsGisin(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CollinsGisinParams &input) override;

        void extra_input_checks(CollinsGisinParams &input) const override;

        void export_whole_tensor(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system);

        void export_one_measurement(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system);

        void export_one_outcome(IOArgumentRange output, CollinsGisinParams &input, MatrixSystem& system);

    };

}
