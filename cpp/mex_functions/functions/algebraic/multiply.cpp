/**
 * multiply.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "multiply.h"
#include "binary_operation_impl.h"
#include "errors.h"

#include "environmental_variables.h"

#include "scenarios/context.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"


#include <cassert>

namespace Moment::mex::functions {

    MultiplyParams::MultiplyParams(SortedInputs&& inputs) : BinaryOperationParams{std::move(inputs)} {
        switch (this->product_type()) {
            case ProductType::OneToOne:
            case ProductType::OneToMany:
            case ProductType::ManyToOne:
            case ProductType::ManyToMany:
            case ProductType::OneToMatrix:
            case ProductType::MatrixToOne:
                break;
            default:
                throw BadParameter{"Currently, multiply is only supported for array inputs, or scalar/matrix products"};
        }
    }

    // Explicitly instantiate binary operation functions:
    template class BinaryOperation<MultiplyParams, MTKEntryPointID::Multiply>;


    RawPolynomial Multiply::one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) {
        assert(this->context_ptr);
        return this->context_ptr->multiply(lhs, rhs);
    }

    namespace {
        template<bool premultiply>
        std::pair<ptrdiff_t, const SymbolicMatrix&>
        inline do_poly_matrix_multiply(const MaintainsMutex::WriteLock& write_lock, MatrixSystem& system,
                                        const RawPolynomial& the_poly, const SymbolicMatrix& the_matrix,
                                        Multithreading::MultiThreadPolicy mt_policy) {
            assert(system.is_locked_write_lock(write_lock));

            auto& symbols = system.Symbols();
            const auto& factory = system.polynomial_factory();

            ptrdiff_t offset = -1;

            auto output_matrix_ptr = [&] { ;
                if constexpr (premultiply) {
                    return the_matrix.pre_multiply(the_poly, factory, symbols, mt_policy);
                } else {
                    return the_matrix.post_multiply(the_poly, factory, symbols, mt_policy);
                }
            }();

            assert(output_matrix_ptr);
            const SymbolicMatrix& output_matrix = *output_matrix_ptr;
            offset = system.push_back(write_lock, std::move(output_matrix_ptr)); // Moving ownership, not object...!
            assert(offset >= 0);
            return std::pair<ptrdiff_t, const SymbolicMatrix&>{offset, output_matrix };
        }
    }

    std::pair<ptrdiff_t, const SymbolicMatrix&>
    Multiply::one_to_matrix(const MaintainsMutex::WriteLock& write_lock,
                            const RawPolynomial& lhs, const SymbolicMatrix& rhs) {
        return do_poly_matrix_multiply<true>(write_lock, *this->ms_ptr, lhs, rhs,
                                             this->settings->get_mt_policy());
    }

    std::pair<ptrdiff_t, const SymbolicMatrix&>
    Multiply::matrix_to_one(const MaintainsMutex::WriteLock& write_lock, const SymbolicMatrix& lhs,
                            const RawPolynomial& rhs) {
        return do_poly_matrix_multiply<false>(write_lock, *this->ms_ptr, rhs, lhs,
                                              this->settings->get_mt_policy());
    }

}
