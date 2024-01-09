/**
 * commutator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "commutator.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "storage_manager.h"

#include "utilities/reporting.h"
#include "export/export_polynomial.h"
#include "export/full_monomial_specification.h"


namespace Moment::mex::functions {
    CommutatorParams::CommutatorParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)},
              matrix_system_key{matlabEngine}, lhs{matlabEngine, "LHS"}, rhs{matlabEngine, "RHS"} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Check type of LHS input
        this->lhs.parse_input(this->inputs[1]);

        // Check type of RHS input
        this->rhs.parse_input(this->inputs[2]);

        // Check type dimensions
        this->resolved_product_type = product_type(this->lhs, this->rhs);
        switch (this->resolved_product_type) {
            default:
            case ProductType::Incompatible:
            case ProductType::OneToMatrix:
            case ProductType::MatrixToOne:
                throw_error(matlabEngine, errors::bad_param,
                            "Incompatible operand types.");
            case ProductType::MismatchedDimensions:
                throw_error(matlabEngine, errors::bad_param,
                            "Operand dimensions must match for element-wise operation.");
            case ProductType::OneToOne:
            case ProductType::OneToMany:
            case ProductType::ManyToOne:
            case ProductType::ManyToMany:

                // Product is okay
                break;
        }

        // Check if commuting or anticommuting
        if (this->flags.contains(u"commute")) {
            this->anticommute = false;
        } else if (this->flags.contains(u"anticommute")) {
            this->anticommute = true;
        }
    }

    namespace {
        template<bool anticommutator>
        void do_pauli_one_to_one(matlab::engine::MATLABEngine& matlabEngine,
                                 const Pauli::PauliMatrixSystem& system,
                                 IOArgumentRange& output, CommutatorParams& input) {
            const auto& context = system.pauliContext;
            auto lhs_one = input.lhs.to_raw_polynomial(system);
            auto rhs_one = input.rhs.to_raw_polynomial(system);

            const auto result = [&]() {
                if constexpr (anticommutator) {
                    return context.anticommutator(lhs_one, rhs_one);
                } else {
                    return context.commutator(lhs_one, rhs_one);
                }
            }();

            const bool is_monomial = (result.size() <= 1);

            matlab::data::ArrayFactory factory;
            // Output result as scalar object
            if (output.size() >= 1) {
                PolynomialExporter exporter{matlabEngine, factory, system.pauliContext,
                                            system.Symbols(), system.polynomial_factory().zero_tolerance};
                auto fms = exporter.sequences(result);
                if (is_monomial) {
                    output[0] = fms.move_to_cell(factory);
                } else {
                    auto output_cell = factory.createCellArray(matlab::data::ArrayDimensions{1, 1});
                    *output_cell.begin() = fms.move_to_cell(factory);
                    output[0] = std::move(output_cell);
                }
            }

            // Record if monomial
            if (output.size() >= 2) {
                output[1] = factory.createScalar<bool>(is_monomial);
            }
        }

        void output_many(matlab::engine::MATLABEngine& matlabEngine,
                         const Pauli::PauliMatrixSystem& system,
                         IOArgumentRange& output,
                         std::span<const RawPolynomial> polynomials,
                         const matlab::data::ArrayDimensions& target_shape) {

            // Monomial, if every entry is monomial
            const bool is_monomial = std::all_of(polynomials.begin(), polynomials.end(),
                                                 [](const RawPolynomial& poly) { return poly.size() <= 1; });

            matlab::data::ArrayFactory factory;
            if (output.size() >= 1) {

                PolynomialExporter exporter{matlabEngine, factory, system.pauliContext,
                                            system.Symbols(), system.polynomial_factory().zero_tolerance};
                if (is_monomial) {
                    auto fms = exporter.monomial_sequence_cell_vector(polynomials, target_shape);
                    output[0] = fms.move_to_cell(factory);
                } else {
                    output[0] = exporter.sequence_cell_vector(polynomials, target_shape);
                }
            }

            if ((output.size() >= 2) && is_monomial) {
                output[1] = factory.createScalar<bool>(is_monomial);
            }
        }

        template<bool anticommutator>
        void do_pauli_one_to_many(matlab::engine::MATLABEngine& matlabEngine,
                                 const Pauli::PauliMatrixSystem& system,
                                 IOArgumentRange& output, CommutatorParams& input) {
            const auto& context = system.pauliContext;
            auto lhs_one = input.lhs.to_raw_polynomial(system);
            auto rhs_many = input.rhs.to_raw_polynomial_array(system);

            std::vector<RawPolynomial> results;
            for (const auto& rhs_one : rhs_many) {
                if constexpr (anticommutator) {
                    results.emplace_back(context.anticommutator(lhs_one, rhs_one));
                } else {
                    results.emplace_back(context.commutator(lhs_one, rhs_one));
                }
            }

            output_many(matlabEngine, system, output, results, input.rhs.shape);
        }

        template<bool anticommutator>
        void do_pauli_many_to_one(matlab::engine::MATLABEngine& matlabEngine,
                                 const Pauli::PauliMatrixSystem& system,
                                 IOArgumentRange& output, CommutatorParams& input) {
            const auto& context = system.pauliContext;
            auto lhs_many = input.lhs.to_raw_polynomial_array(system);
            auto rhs_one = input.rhs.to_raw_polynomial(system);

            std::vector<RawPolynomial> results;
            for (const auto& lhs_one : lhs_many) {
                if constexpr (anticommutator) {
                    results.emplace_back(context.anticommutator(lhs_one, rhs_one));
                } else {
                    results.emplace_back(context.commutator(lhs_one, rhs_one));
                }
            }

            output_many(matlabEngine, system, output, results, input.lhs.shape);
        }

        template<bool anticommutator>
        void do_pauli_many_to_many(matlab::engine::MATLABEngine& matlabEngine,
                                 const Pauli::PauliMatrixSystem& system,
                                 IOArgumentRange& output, CommutatorParams& input) {
            const auto& context = system.pauliContext;
            auto lhs_many = input.lhs.to_raw_polynomial_array(system);
            auto rhs_many = input.rhs.to_raw_polynomial_array(system);

            assert(lhs_many.size() == rhs_many.size()); // Was checked earlier.
            std::vector<RawPolynomial> results;
            for (size_t index = 0, count = lhs_many.size(); index < count; ++index) {
                if constexpr (anticommutator) {
                    results.emplace_back(context.anticommutator(lhs_many[index], rhs_many[index]));
                } else {
                    results.emplace_back(context.commutator(lhs_many[index], rhs_many[index]));
                }
            }

            output_many(matlabEngine, system, output, results, input.lhs.shape);
        }

        template<bool anticommutator>
        void do_pauli_calculate(matlab::engine::MATLABEngine& matlabEngine,
                      const Pauli::PauliMatrixSystem& system,
                      IOArgumentRange& output, CommutatorParams& input) {
            switch (input.resolved_product_type) {
                case ProductType::OneToOne:
                    do_pauli_one_to_one<anticommutator>(matlabEngine, system, output, input);
                break;
                case ProductType::OneToMany:
                    do_pauli_one_to_many<anticommutator>(matlabEngine, system, output, input);
                break;
                case ProductType::ManyToOne:
                    do_pauli_many_to_one<anticommutator>(matlabEngine, system, output, input);
                break;
                case ProductType::ManyToMany:
                    do_pauli_many_to_many<anticommutator>(matlabEngine, system, output, input);
                break;
                default:
                    throw_error(matlabEngine, errors::internal_error, "Unexpected product type");
            }
        }

    }

    Commutator::Commutator(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"commute");
        this->flag_names.emplace(u"anticommute");

        this->mutex_params.add_mutex(u"commute", u"anticommute");
    }

    void Commutator::operator()(IOArgumentRange output, CommutatorParams& input) {
        // Get ownership of matrix system
        auto ms_ptr = input.matrix_system_key(this->storageManager);

        // Context doesn't change after construction, and no symbols/matrices are read/written, so no read lock needed.

        const auto * const pauli_ms_ptr = dynamic_cast<const Pauli::PauliMatrixSystem*>(ms_ptr.get());
        if (pauli_ms_ptr) {
            if (input.anticommute) {
                do_pauli_calculate<true>(this->matlabEngine, *pauli_ms_ptr, output, input);
            } else {
                do_pauli_calculate<false>(this->matlabEngine, *pauli_ms_ptr, output, input);
            };
        } else {
            throw_error(this->matlabEngine, errors::internal_error,
                        "`commutator` currently only implemented for Pauli scenarios.");
        }

    }

}