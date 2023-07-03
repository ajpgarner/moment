/**
 * inflation_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_collins_gisin.h"

#include "inflation_matrix_system.h"
#include "inflation_context.h"

#include "utilities/multi_dimensional_index_iterator.h"


namespace Moment::Inflation {

    namespace {
        [[nodiscard]] constexpr std::vector<size_t> make_dimensions(const InflationContext& context) {

            const size_t observable_count = context.observable_variant_count();
            std::vector<size_t> output;
            output.reserve(observable_count);

            for (const auto& observable : context.Observables()) {
                // (outcomes - 1) + 1; for operators, but also with identity.
                std::fill_n(std::back_inserter(output), observable.variant_count, observable.outcomes);
            }
            assert(output.size() == observable_count);

            return output;
        }
    }

    InflationCollinsGisin::InflationCollinsGisin(const InflationMatrixSystem &matrixSystem)
            : CollinsGisin{matrixSystem.Context(), matrixSystem.Symbols(),
                           make_dimensions(matrixSystem.InflationContext())},
              context{matrixSystem.InflationContext()} {

        // Prepare global measurement -> party/measurement data.
        this->gmIndex.reserve(this->DimensionCount);
        size_t d = 0;
        for (const auto& observable : context.Observables()) {
            const size_t op_num = observable.outcomes-1;
            for (const auto& variant : observable.variants) {

                this->dimensionInfo[d].op_ids.reserve(this->Dimensions[d]);
                this->dimensionInfo[d].op_ids.emplace_back(-1); // index 0 is always no-op.

                for (size_t outcome = 0; outcome < op_num; ++outcome) {
                    this->dimensionInfo[d].op_ids.emplace_back(variant.operator_offset + outcome);
                }

                this->gmIndex.emplace_back(d, 1, op_num);
                ++d;
            }
        }



        if (this->StorageType == TensorStorageType::Explicit) {
            // Make from tensor indices
            for (const auto &cgIndex: MultiDimensionalIndexRange<true>{Dimensions}) {
                this->data.emplace_back(*this, cgIndex);
            }

            // Try to find symbols
            this->do_initial_symbol_search();
        }
    }

}
