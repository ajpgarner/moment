/**
 * tensor_conversion.cpp
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "tensor_conversion.h"
#include "locality_context.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace Moment::Locality {

    namespace {
        [[nodiscard]] TensorConvertor::TensorType set_up_tensor(const LocalityContext& context) {

            const auto& opp = context.operators_per_party();
            std::vector<size_t> tensor_dimensions;
            tensor_dimensions.reserve(opp.size());
            std::transform(opp.cbegin(), opp.cend(), std::back_inserter(tensor_dimensions), [](const size_t ops) {
                return ops+1;
            });
            return TensorConvertor::TensorType{std::move(tensor_dimensions)};
        }

        std::vector<double> cg_to_fc_matrix(TensorConvertor::TensorType tensor_info,
                                            const std::span<const double> cg_tensor) {
            std::vector<double> output(tensor_info.ElementCount, 0.0);

            // Calculate marginals
            std::vector<double> alice_marginals(tensor_info.Dimensions[0], 0.0); // sum over b for, e, a0, a1, ...
            std::vector<double> bob_marginals(tensor_info.Dimensions[1], 0.0);   // sum over a for, e, b0, b1, ...


            // e, b: sum over B only
            std::array<size_t, 2> index{0, 0};
            for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                const size_t offset = tensor_info.index_to_offset(index);
                alice_marginals[0] += cg_tensor[offset];
            }

            // a, e
            index[1] = 0;
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                const size_t offset = tensor_info.index_to_offset(index);
                bob_marginals[0] += cg_tensor[offset];
            }

            // a, b
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                    const size_t offset = tensor_info.index_to_offset(index);

                    alice_marginals[index[0]] += cg_tensor[offset];
                    bob_marginals[index[1]] += cg_tensor[offset];
                }
            }

            // Joint marginal
            const double central_sum = std::reduce(alice_marginals.cbegin()+1, alice_marginals.cend(), 0.0);

            // Constant term
            output[0] = cg_tensor[0] + (alice_marginals[0]/2.0) + (bob_marginals[0]/2.0) + (central_sum / 4.0);

            // a- terms
            index[1] = 0;
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                const size_t offset = tensor_info.index_to_offset(index);
                output[offset] = (cg_tensor[offset] /2.0) + (alice_marginals[index[0]] / 4.0);
            }

            // -b terms
            index[0] = 0;
            for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                const size_t offset = tensor_info.index_to_offset(index);
                output[offset] = (cg_tensor[offset] /2.0) + (bob_marginals[index[1]] / 4.0);
            }

            // ab terms
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                    const size_t offset = tensor_info.index_to_offset(index);
                    output[offset] = cg_tensor[offset] / 4.0;
                }
            }

            return output;
        }

        std::vector<double> fc_to_cg_matrix(TensorConvertor::TensorType tensor_info,
                                            const std::span<const double> fc_tensor) {
            std::vector<double> output(tensor_info.ElementCount, 0.0);

            // Calculate marginals
            std::vector<double> alice_marginals(tensor_info.Dimensions[0], 0.0); // sum over b for, e, a0, a1, ...
            std::vector<double> bob_marginals(tensor_info.Dimensions[1], 0.0);   // sum over a for, e, b0, b1, ...


            // e, b: sum over B only
            std::array<size_t, 2> index{0, 0};
            for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                const size_t offset = tensor_info.index_to_offset(index);
                alice_marginals[0] += fc_tensor[offset];
            }

            // a, e
            index[1] = 0;
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                const size_t offset = tensor_info.index_to_offset(index);
                bob_marginals[0] += fc_tensor[offset];
            }

            // a, b
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                    const size_t offset = tensor_info.index_to_offset(index);

                    alice_marginals[index[0]] += fc_tensor[offset];
                    bob_marginals[index[1]] += fc_tensor[offset];
                }
            }

            // Joint marginal
            const double central_sum = std::reduce(alice_marginals.cbegin()+1, alice_marginals.cend(), 0.0);

            // Constant term
            output[0] = fc_tensor[0] + central_sum - alice_marginals[0] - bob_marginals[0];

            // a- terms
            index[1] = 0;
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                const size_t offset = tensor_info.index_to_offset(index);
                output[offset] = (2*fc_tensor[offset]) - (2.0 * alice_marginals[index[0]]);
            }

            // -b terms
            index[0] = 0;
            for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                const size_t offset = tensor_info.index_to_offset(index);
                output[offset] = (2*fc_tensor[offset]) - (2.0 * bob_marginals[index[1]]);
            }

            // ab terms
            for (index[0] = 1; index[0] < tensor_info.Dimensions[0]; ++index[0]) {
                for (index[1] = 1; index[1] < tensor_info.Dimensions[1]; ++index[1]) {
                    const size_t offset = tensor_info.index_to_offset(index);
                    output[offset] = 4.0 * fc_tensor[offset];
                }
            }
            return output;
        }
    }

    TensorConvertor::TensorConvertor(const LocalityContext& context)
    : context{context}, tensor_info{set_up_tensor(context)} {
        // Check outcomes per measurement
        const auto& opm = context.outcomes_per_measurement();
        if (!std::all_of(opm.cbegin(), opm.cend(), [](const size_t outcomes) {
            return outcomes == 2;
        } )) {
            throw std::logic_error{"Full correlator <-> Collins-Gisin conversion is only possible for binary measurements."};
        }

        // For now, limit to bipartite
        if (context.Parties.size() != 2) {
            throw std::runtime_error{"Currently only bipartite scenarios are supported."};
        }

    }

    std::vector<double> TensorConvertor::full_correlator_to_collins_gisin(std::span<const double> fc_tensor) const {
        if (fc_tensor.size() != this->tensor_info.ElementCount) {
            throw std::invalid_argument("The input CG tensor view was the wrong size.");
        }

        // Special case: bipartite
        if (tensor_info.DimensionCount == 2) {
            return fc_to_cg_matrix(this->tensor_info, fc_tensor);
        }

        // For now, limit to bipartite
        throw std::runtime_error{"Currently only full-correlator matrices are supported."};
    }

    std::vector<double> TensorConvertor::collins_gisin_to_full_correlator(std::span<const double> cg_tensor) const {
        if (cg_tensor.size() != this->tensor_info.ElementCount) {
            throw std::invalid_argument("The input CG tensor view was the wrong size.");
        }

        // Special case: bipartite
        if (tensor_info.DimensionCount == 2) {
            return cg_to_fc_matrix(this->tensor_info, cg_tensor);
        }

        // For now, limit to bipartite
        throw std::runtime_error{"Currently only full-correlator matrices are supported."};
    }

}