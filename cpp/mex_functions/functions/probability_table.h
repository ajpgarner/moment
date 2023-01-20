/**
 * probability_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"
#include "scenarios/inflation/observable_variant_index.h"
#include "scenarios/locality/measurement.h"

namespace Moment {
    namespace Locality {
        class LocalityMatrixSystem;
    }
    namespace Inflation {
        class InflationMatrixSystem;
    }
}

namespace Moment::mex::functions {

    struct ProbabilityTableParams : public SortedInputs {
    public:
        /** Do we export the entire probability table, or just one entry... */
        enum struct ExportMode {
            WholeTable,
            OneMeasurement,
            OneOutcome
        } export_mode = ExportMode::WholeTable;


        struct RawTriplet {
            size_t first, second, third;
            RawTriplet() = default;
            RawTriplet(size_t a, size_t b, size_t c) : first{a}, second{b}, third{c} { }
        };

        /** The reference to the matrix system */
        uint64_t matrix_system_key = 0;

        std::vector<RawTriplet> requested_indices{};

        /** Interpret requested indices as PM index */
        [[nodiscard]] std::vector<Locality::PMIndex> requested_measurement() const;

        /** Interpret requested indices as PMO index */
        [[nodiscard]] std::vector<Locality::PMOIndex> requested_outcome() const;

        /** Interpret requested indices as OV index */
        [[nodiscard]] std::vector<Inflation::OVIndex> requested_observables() const;

        /** Interpret requested indices as OVO index */
        [[nodiscard]] std::vector<Inflation::OVOIndex> requested_ovo() const;

    public:
        explicit ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);
    };

class ProbabilityTable : public ParameterizedMexFunction<ProbabilityTableParams, MEXEntryPointID::ProbabilityTable> {
    public:
        explicit ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

protected:
    void operator()(IOArgumentRange output, ProbabilityTableParams &input) override;

    void extra_input_checks(ProbabilityTableParams &input) const override;

private:
        void export_locality(IOArgumentRange output,
                             ProbabilityTableParams& input,
                             const Locality::LocalityMatrixSystem& lms);

        void export_inflation(IOArgumentRange output,
                              ProbabilityTableParams& input,
                              const Inflation::InflationMatrixSystem& ims);

    };

}