/**
 * locality_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_collins_gisin.h"

#include "locality_context.h"
#include "locality_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment::Locality {

    namespace {
        [[nodiscard]] constexpr std::vector<size_t> make_dimensions(const std::vector<size_t>& oc) {
            std::vector<size_t> output;
            output.reserve(oc.size());
            for (auto val : oc) {
                output.push_back(val+1);
            }
            return output;
        }

        OperatorSequence make_op_seq(const LocalityContext& context, std::span<const size_t> index) {
            sequence_storage_t ops;
            for (size_t p = 0, pMax = context.Parties.size(); p < pMax; ++p) {
                if (0 == index[p]) {
                    continue;
                }
                ops.emplace_back(context.Parties[p][index[p]-1]);
            }

            return OperatorSequence{std::move(ops), context};
        }

    }

    LocalityCollinsGisin::LocalityCollinsGisin(const LocalityMatrixSystem &matrixSystem)
        : CollinsGisin{make_dimensions(matrixSystem.localityContext.operators_per_party())},
          context{matrixSystem.localityContext} {

        // Prepare global measurement -> party/measurement data
        for (const auto& party : context.Parties) {
            size_t party_offset = 1; // [Offset 0 is reserved for identity operator.]
            for (const auto& mmt : party.Measurements) {
                this->gmIndex.emplace_back(static_cast<size_t>(party.id()),
                                           party_offset,
                                           static_cast<size_t>(mmt.num_operators()));
                party_offset += static_cast<size_t>(mmt.num_operators());
            }
        }

        // Build array in column-major format, for quick export to matlab.
        for (const auto& cgIndex : MultiDimensionalIndexRange<true>{Dimensions}) {
            this->sequences.emplace_back(make_op_seq(this->context, cgIndex));
        }

        // Try to find symbols
        this->do_initial_symbol_search(matrixSystem.Symbols());

    }

}
