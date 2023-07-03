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

            const size_t observable_count = context.Observables().size();
            std::vector<size_t> output;
            output.reserve(observable_count);

            // TODO: ICG

            return output;
        }

        OperatorSequence make_op_seq(const InflationContext& context, std::span<const size_t> index) {
            sequence_storage_t ops;
            // TOTO: ICG

//            for (size_t p = 0, pMax = context.Parties.size(); p < pMax; ++p) {
//                if (0 == index[p]) {
//                    continue;
//                }
//                ops.emplace_back(context.Parties[p][index[p]-1]);
//            }

            return OperatorSequence{std::move(ops), context};
        }

    }

    InflationCollinsGisin::InflationCollinsGisin(const InflationMatrixSystem &matrixSystem)
            : CollinsGisin{matrixSystem.Context(), matrixSystem.Symbols(),
                           make_dimensions(matrixSystem.InflationContext())},
              context{matrixSystem.InflationContext()} {


//        // Prepare global measurement -> party/measurement data
//        for (const auto& observable : context.Observables()) {
//            size_t party_offset = 0;
//            for (const auto& mmt : party.Measurements) {
//                this->gmIndex.emplace_back(static_cast<size_t>(party.id()),
//                                           party_offset,
//                                           static_cast<size_t>(mmt.num_operators()));
//                party_offset += static_cast<size_t>(mmt.num_operators());
//            }
//        }

        // TODO: Decide, do we want 'AxAy' statistics, or just 'AxBy...' statistics?
//
//        const auto& symbol_table = matrixSystem.Symbols();
//
//
////        // Build array in column-major format, for quick export to matlab.
////        for (const auto& cgIndex : MultiDimensionalIndexRange<true>{Dimensions}) {
////            this->sequences.emplace_back(make_op_seq(this->context, cgIndex));
////        }
//
//        // Try to find symbols
//        this->do_initial_symbol_search();

    }

}
