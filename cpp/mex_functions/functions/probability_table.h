/**
 * probability_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"
#include "operators/inflation/observable_variant_index.h"
#include "operators/locality/measurement.h"

namespace NPATK {
    class LocalityMatrixSystem;
    class InflationMatrixSystem;
}

namespace NPATK::mex::functions {

    struct ProbabilityTableParams : public SortedInputs {
    public:
        /** Do we export the entire probability table, or just one entry... */
        enum struct ExportMode {
            WholeTable,
            OneMeasurement,
            OneOutcome,
            OneInflationObservable,
            OneInflationOutcome
        } export_mode = ExportMode::WholeTable;

        /** The reference to the matrix system */
        uint64_t matrix_system_key = 0;

        /** The PM index to export */
        std::vector<PMIndex> requested_measurement{};

        /** The PMO index to export */
        std::vector<PMOIndex> requested_outcome{};

        /** The OV index to export */
        std::vector<OVIndex> requested_observables{};

        /** The OVO index to export */
        std::vector<OVOIndex> requested_ovo{};

        bool inflation_mode = false;
    public:
        explicit ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);
    };

    class ProbabilityTable : public MexFunction {
    public:
        explicit ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

    private:
        void export_locality(IOArgumentRange output,
                             ProbabilityTableParams& input,
                             const LocalityMatrixSystem& lms);

        void export_inflation(IOArgumentRange output,
                              ProbabilityTableParams& input,
                              const InflationMatrixSystem& ims);

    };

}