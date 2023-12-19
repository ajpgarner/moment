/**
 * probability_table.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "mtk_function.h"

#include "scenarios/inflation/observable_variant_index.h"
#include "scenarios/locality/party_measurement_index.h"

#include "import/matrix_system_id.h"
#include "import/read_measurement_indices.h"

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions {

    struct ProbabilityTableParams : public SortedInputs {
    public:
        /** Do we export the entire probability table, or just one entry... */
        enum struct ExportShape {
            /** Export probability tensor in its entirety. */
            WholeTensor,
            /** Export slice representing measurement. */
            OneMeasurement,
            /** Export slice representing single outcome. */
            OneOutcome
        } export_shape = ExportShape::WholeTensor;

        /** How do we want to export */
        enum struct OutputMode {
            /** Export specification of polynomials. */
            OperatorSequences,
            /** Export specification of polynomials, but also include symbol info. */
            OperatorSequencesWithSymbolInfo,
            /** Export as cell array of symbols. */
            Symbols
        } output_mode = OutputMode::Symbols;

        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** Measurements that we get all outcomes for */
        std::vector<RawIndexPair> free{};

        /** Measurements that we fix outcomes for */
        std::vector<RawIndexTriplet> fixed{};

    public:
        explicit ProbabilityTableParams(SortedInputs&& structuredInputs);
    };

class ProbabilityTable : public ParameterizedMTKFunction<ProbabilityTableParams, MTKEntryPointID::ProbabilityTable> {
    public:
        explicit ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

protected:
    void operator()(IOArgumentRange output, ProbabilityTableParams &input) override;


private:
        void export_whole_tensor(IOArgumentRange output,
                                 ProbabilityTableParams& input,
                                 MatrixSystem& system);

        void export_one_measurement(IOArgumentRange output,
                                    ProbabilityTableParams& input,
                                    MatrixSystem& system);

        void export_one_outcome(IOArgumentRange output,
                                ProbabilityTableParams& input,
                                MatrixSystem& system);

    };

}