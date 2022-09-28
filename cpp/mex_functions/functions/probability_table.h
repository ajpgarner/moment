/**
 * probability_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"
#include "operators/locality/measurement.h"


namespace NPATK::mex::functions {

    struct ProbabilityTableParams : public SortedInputs {
    public:
        /** Do we export the entire probability table, or just one entry... */
        enum struct ExportMode {
            WholeTable,
            OneMeasurement,
            OneOutcome
        } export_mode = ExportMode::WholeTable;

        /** The reference to the matrix system */
        uint64_t matrix_system_key = 0;

        /** The PM index to export */
        std::vector<PMIndex> requested_measurement{};

        /** The PMO index to export */
        std::vector<PMOIndex> requested_outcome{};

    public:
        explicit ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);
    };

    class ProbabilityTable : public MexFunction {
    public:
        explicit ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };

}