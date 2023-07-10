/**
 * make_explicit.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "make_explicit.h"

#include "storage_manager.h"

#include "export/export_polynomial.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_probability_tensor.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_probability_tensor.h"

#include "symbolic/polynomial_factory.h"

#include "utilities/float_utils.h"

#include <functional>
#include <numeric>
#include <tuple>
#include <sstream>

namespace Moment::mex::functions {

    namespace {
        [[nodiscard]] std::tuple<ProbabilityTensorRange, ProbabilityTensor::ElementView, const MaintainsTensors&>
        get_slice_and_norm(matlab::engine::MATLABEngine& matlabEngine, const MakeExplicitParams& input,
                           MatrixSystem& system, decltype(system.get_read_lock())& lock) {

            if (auto* lmsPtr = dynamic_cast<Locality::LocalityMatrixSystem*>(&system); lmsPtr != nullptr) {
                lmsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = lmsPtr->LocalityProbabilityTensor();

                PMConvertor pmReader{matlabEngine, lmsPtr->localityContext, true};
                auto free_mmts = pmReader.read_pm_index_list(input.free_indices);
                auto fixed_mmts = pmReader.read_pmo_index_list(input.fixed_indices);

                try {
                    return std::tuple<ProbabilityTensorRange, ProbabilityTensor::ElementView, const MaintainsTensors&>(
                            pt.measurement_to_range(free_mmts, fixed_mmts),
                            pt.outcome_to_element(fixed_mmts),
                            *lmsPtr);
                } catch (const Moment::errors::BadPTError& pte) {
                    throw_error(matlabEngine, errors::bad_param, pte.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            if (auto* imsPtr = dynamic_cast<Inflation::InflationMatrixSystem*>(&system); imsPtr != nullptr) {
                imsPtr->RefreshProbabilityTensor(lock);
                const auto& pt = imsPtr->InflationProbabilityTensor();

                OVConvertor ovReader{matlabEngine, imsPtr->InflationContext(), true};
                auto free_mmts = ovReader.read_ov_index_list(input.free_indices);
                auto fixed_mmts = ovReader.read_ovo_index_list(input.fixed_indices);

                try {
                    return std::tuple<ProbabilityTensorRange, ProbabilityTensor::ElementView, const MaintainsTensors&>(
                            pt.measurement_to_range(free_mmts, fixed_mmts),
                            pt.outcome_to_element(fixed_mmts),
                            *imsPtr);
                } catch (const Moment::errors::BadPTError& pte) {
                    throw_error(matlabEngine, errors::bad_param, pte.what());
                } catch (const std::exception& e) {
                    throw_error(matlabEngine, errors::internal_error, e.what());
                }
            }

            throw_error(matlabEngine, errors::bad_param, "Matrix system must be a locality or inflation system.");
        }


    }

    MakeExplicitParams::MakeExplicitParams(SortedInputs &&structuredInputs)
           : SortedInputs(std::move(structuredInputs)) {
        // Get system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Get output mode if set
        if (this->flags.contains(u"symbols")) {
            this->output_type = OutputType::SymbolCell;
        } else if (this->flags.contains(u"polynomials")) {
            this->output_type = OutputType::Polynomial;
        }

        // Get if we are implicitly in conditional mode?
        if (this->flags.contains(u"conditional")) {
            this->is_conditional = true;
        } else {
            this->is_conditional = false;
        }

        // Get measurement
        size_t value_input_index = this->inputs.size()-1;
        if (value_input_index == 2) {
            std::tie(this->free_indices, this->fixed_indices) = read_pairs_and_triplets(this->matlabEngine, inputs[1]);
        } else {
            std::tie(this->free_indices, this->fixed_indices)
                = read_pairs_and_triplets(this->matlabEngine, inputs[1], inputs[2]);
        }

        // Get values
        this->values = read_as_vector<double>(matlabEngine, this->inputs[value_input_index]);
    }


    MakeExplicit::MakeExplicit(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 4;
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"conditional");

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"polynomials");
        this->mutex_params.add_mutex({u"symbols", u"polynomials", u"all"});
    }

    void MakeExplicit::extra_input_checks(MakeExplicitParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }

        if (!this->quiet && input.is_conditional && input.free_indices.empty()) {
            print_warning(matlabEngine,
                          "Conditional probability export was requested, but no fixed outcomes were specified.");
        }

    }

    void MakeExplicit::operator()(IOArgumentRange output, MakeExplicitParams &input) {
        // Get matrix system ptr from storage
        std::shared_ptr<MatrixSystem> matrixSystemPtr = [&]() { ;
            try {
                return this->storageManager.MatrixSystems.get(input.matrix_system_key);
            } catch (const Moment::errors::persistent_object_error &poe) {
                std::stringstream errSS;
                errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key
                      << std::dec;
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
        }();
        assert(matrixSystemPtr); // ^-- should throw if not found

        // Lock and get slice / cast matrix to form where probability tensor is defined.
        auto lock = matrixSystemPtr->get_read_lock();
        auto [slice, norm, matrixSystem] = get_slice_and_norm(this->matlabEngine, input, *matrixSystemPtr, lock);

        // Check dimensions of RHS; add implicit final value if necessary; check that values sum to unity.
        this->check_count(matrixSystem, slice.size(), input);

        // Get probability tensor
        const auto& pt = matrixSystem.ProbabilityTensor();

        // Get rules
        std::vector<Polynomial> rules;
        if (input.conditional()) {
            rules = pt.explicit_value_rules(slice, norm, input.values);
        } else {
            rules = pt.explicit_value_rules(slice, input.values);
        }

        // Export rule polynomials
        PolynomialExporter exporter{this->matlabEngine, matrixSystem.Symbols(),
                                    matrixSystem.polynomial_factory().zero_tolerance};
        switch (input.output_type) {
            case MakeExplicitParams::OutputType::SymbolCell:
                output[0] = exporter.symbol_cell_vector(rules);
                break;
            case MakeExplicitParams::OutputType::Polynomial:
                output[0] = exporter.sequence_cell_vector(rules, true);
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
        }
    }

    void MakeExplicit::check_count(const MatrixSystem &system, const size_t slice_size, MakeExplicitParams &input) {
        const size_t value_size = input.values.size();
        const bool can_be_normalized = input.is_conditional || input.fixed_indices.empty();
        const double zero_tolerance = system.polynomial_factory().zero_tolerance;

            if (slice_size != value_size) {
                // If we are a complete probability distribution, we can infer final value:
                if (can_be_normalized && (slice_size == value_size + 1)) {
                    const double all_but_one = std::reduce(input.values.cbegin(), input.values.cend(), 0.0, std::plus{});
                    if (!quiet && definitely_greater_than(all_but_one, 1.0, zero_tolerance)) {
                        std::stringstream warnSS;
                        warnSS << "Supplied probabilities summed to " << all_but_one << ", which is larger than unity.";
                        print_warning(matlabEngine, warnSS.str());
                    }
                    input.values.emplace_back(all_but_one);
                } else {
                    std::stringstream errSS;
                    errSS << "Expected " << slice_size << " values to define ";
                    if (!can_be_normalized) {
                        errSS << "(possibly subnormal)";
                    }
                    errSS << " probability distribution, but only " << value_size << " were provided.";
                    throw_error(matlabEngine, errors::bad_param, errSS.str());
                }
            } else if (!quiet) {
                const double total = std::reduce(input.values.cbegin(), input.values.cend(), 0.0, std::plus{});

                if (can_be_normalized) {
                    if (!approximately_equal(total, 1.0, zero_tolerance)) {
                        std::stringstream warnSS;
                        warnSS << "Values of probability distribution add up to " << total << " (unity expected).";
                        print_warning(matlabEngine, warnSS.str());
                    }
                } else {
                    if (definitely_greater_than(total, 1.0, zero_tolerance)) {
                        std::stringstream warnSS;
                        warnSS << "Supplied probabilities summed to " << total << ", which is larger than unity.";
                        print_warning(matlabEngine, warnSS.str());
                    }
                }
            }

    }
}