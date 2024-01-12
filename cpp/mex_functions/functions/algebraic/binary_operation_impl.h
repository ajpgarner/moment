/**
 * binary_operation_impl.h
 *
 * Should be included (and then instantiated) in any class deriving from BinaryOperation.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "binary_operation.h"

#include "export/export_polynomial.h"
#include "export/full_monomial_specification.h"

#include "utilities/reporting.h"

#include "matrix/symbolic_matrix.h"
#include "matrix_system/matrix_system.h"

#include <cassert>

namespace Moment::mex::functions {

    template<std::derived_from<BinaryOperationParams> op_param_t, MTKEntryPointID entry_point_id>
    void BinaryOperation<op_param_t, entry_point_id>::operator()(IOArgumentRange output, op_param_t& input) {
        // Get handle to matrix system (or throw error trying)...
        std::shared_ptr<MatrixSystem> matrix_system_ptr = input.matrix_system_key(this->storageManager);

        // Acquire matrix system, context and polynomial factory pointers.
        this->ms_ptr = matrix_system_ptr.get();
        this->context_ptr = &(matrix_system_ptr->Context());
        this->pf_ptr = &(matrix_system_ptr->polynomial_factory());

        // Call for any additional parameter set-up (e.g. derived system types)
        this->additional_setup(input);

        // Switch through types
        try {
            switch (input.product_type()) {
                case ProductType::OneToOne:
                case ProductType::OneToMany:
                case ProductType::ManyToOne:
                case ProductType::ManyToMany:
                    this->polynomial_by_polynomial(output, input);
                    break;
                case ProductType::OneToMatrix:
                case ProductType::MatrixToOne:
                    this->polynomial_by_matrix(output, input);
                    break;
                case ProductType::MatrixToMatrix:
                    this->matrix_by_matrix(output, input);
                    break;
                default:
                    throw BinaryOperationException{"Unexpected product type."};
            }
        } catch (const BinaryOperationException& boe) {
            // Forward error to MATLAB
            throw_error(this->matlabEngine, errors::internal_error, std::string{boe.what()});
        }

        // ~matrix_system_ptr
    }

    template<std::derived_from<BinaryOperationParams> op_param_t, MTKEntryPointID entry_point_id>
    void BinaryOperation<op_param_t, entry_point_id>::polynomial_by_polynomial(IOArgumentRange output,
                                                                               BinaryParams &input) {

        // Prepare to received calculated polynomials and their shape
        std::vector<RawPolynomial> output_polynomials;
        std::vector<size_t> output_shape;

        const auto& system = *this->ms_ptr;
        // Do operation
        switch (input.product_type()) {
            case ProductType::OneToOne:
                output_polynomials.emplace_back(this->one_to_one(input.lhs.to_raw_polynomial(system),
                                                                 input.rhs.to_raw_polynomial(system)));
                output_shape.emplace_back(1);
                output_shape.emplace_back(1);
                break;
            case ProductType::OneToMany:
                output_polynomials = this->one_to_many(input.lhs.to_raw_polynomial(system),
                                                       input.rhs.to_raw_polynomial_array(system));
                std::copy(input.rhs.shape.cbegin(), input.rhs.shape.cend(), std::back_inserter(output_shape));
                break;
            case ProductType::ManyToOne:
                output_polynomials = this->many_to_one(input.lhs.to_raw_polynomial_array(system),
                                                       input.rhs.to_raw_polynomial(system));
                std::copy(input.lhs.shape.cbegin(), input.lhs.shape.cend(), std::back_inserter(output_shape));
                break;
            case ProductType::ManyToMany:
                output_polynomials = this->many_to_many(input.lhs.to_raw_polynomial_array(system),
                                                        input.rhs.to_raw_polynomial_array(system));
                std::copy(input.lhs.shape.cbegin(), input.lhs.shape.cend(), std::back_inserter(output_shape));
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unexpected product type.");
        }

        // Check if output is monomial (true, if every entry is monomial)
        const bool is_monomial = std::all_of(output_polynomials.begin(), output_polynomials.end(),
                                             [](const RawPolynomial& poly) { return poly.size() <= 1; });

        // Do output
        matlab::data::ArrayFactory factory;
        if (output.size() >= 1) {
            PolynomialExporter exporter{this->matlabEngine, factory, *this->context_ptr,
                                        this->ms_ptr->Symbols(),
                                        this->ms_ptr->polynomial_factory().zero_tolerance};
            if (is_monomial) {
                auto fms = exporter.monomial_sequence_cell_vector(output_polynomials, output_shape);
                output[0] = fms.move_to_cell(factory);
            } else {
                output[0] = exporter.sequence_cell_vector(output_polynomials, output_shape);
            }
        }

        if (output.size() >= 2) {
            output[1] = factory.createScalar<bool>(is_monomial);
        }
    }

    namespace {
        inline void do_matrix_info_export(matlab::engine::MATLABEngine& engine, IOArgumentRange output,
                                          const MatrixSystem& system, ptrdiff_t matrix_offset,
                                          const SymbolicMatrix& matrix) {
            matlab::data::ArrayFactory factory;

            // Matrix ID
            if (output.size() >= 1) {
                output[0] = factory.createScalar<int64_t>(matrix_offset);
            }

            // Matrix dimension
            if (output.size() >= 2) {
                output[1] = factory.createScalar<size_t>(matrix.Dimension());
            }

            // Is matrix monomial?
            if (output.size() >= 3) {
                output[2] = factory.createScalar<bool>(matrix.is_monomial());
            }

            // Is matrix Hermitian?
            if (output.size() >= 4) {
                output[3] = factory.createScalar<bool>(matrix.Hermitian());
            }
        }
    }

    template<std::derived_from<BinaryOperationParams> op_param_t, MTKEntryPointID entry_point_id>
    void BinaryOperation<op_param_t, entry_point_id>::polynomial_by_matrix(IOArgumentRange output,
                                                                           BinaryParams &input) {
        assert(this->ms_ptr);

        const bool matrix_lhs = (input.product_type() == ProductType::MatrixToOne);

        auto& system = *this->ms_ptr;
        auto read_lock = system.get_read_lock();

        // Get inputs
        const SymbolicMatrix& input_matrix = matrix_lhs ? input.lhs.to_matrix(system)
                                                        : input.rhs.to_matrix(system);
        const RawPolynomial input_polynomial = matrix_lhs ? input.rhs.to_raw_polynomial(system)
                                                          : input.lhs.to_raw_polynomial(system);

        // In principle, matrices do not change after construction; and shared_ptr to matrix system prevents deletion,
        // such that pointers/references cannot be invalidated, even if another thread races with this one:
        read_lock.unlock();

        // This unnecessarily contends the lock /but/ in practice the MATLAB implementation is not very parallel, and so
        // is not worth the added complexity to defer the lock until after the matrix is calculated (but before it is
        // added to the matrix system
        auto write_lock = system.get_write_lock();

        // Do calculation:
        auto [matrix_offset, matrix_ref] = matrix_lhs ? this->matrix_to_one(write_lock, input_matrix, input_polynomial)
                                                      : this->one_to_matrix(write_lock, input_polynomial, input_matrix);

        write_lock.unlock();

        // Output matrix info
        do_matrix_info_export(this->matlabEngine, output, system, matrix_offset, matrix_ref);

    }

    template<std::derived_from<BinaryOperationParams> op_param_t, MTKEntryPointID entry_point_id>
    void BinaryOperation<op_param_t, entry_point_id>::matrix_by_matrix(IOArgumentRange output, BinaryParams &input) {
        assert(this->ms_ptr);

        // Get inputs
        auto& system = *this->ms_ptr;
        auto read_lock = system.get_read_lock();
        const SymbolicMatrix& lhs_matrix = input.lhs.to_matrix(system);
        const SymbolicMatrix& rhs_matrix = input.rhs.to_matrix(system);
        read_lock.unlock();

        // Do calculation:
        auto write_lock = system.get_write_lock();
        auto [matrix_offset, matrix_ref] = this->matrix_to_matrix(write_lock, lhs_matrix, rhs_matrix);

        write_lock.unlock();

        // Output matrix info
        do_matrix_info_export(this->matlabEngine, output, system, matrix_offset, matrix_ref);

    }


}