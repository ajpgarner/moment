/**
 * substituted_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "substituted_matrix.h"

#include "symbolic/polynomial_factory.h"

#include "multithreading/matrix_transformation_worker.h"

#include <memory>
#include <sstream>

namespace Moment {
    namespace {
       inline SymbolTable& assert_symbols(SymbolTable& symbols, const SymbolicMatrix& the_source) {
            assert(&symbols == &the_source.symbols);
            return symbols;
        }

        template<typename output_elem, typename input_elem, typename functor_t>
        std::unique_ptr<SquareMatrix<output_elem>>
        inline do_reduction(const SquareMatrix<input_elem>& input,
                     const Multithreading::MultiThreadPolicy mt_policy,
                     size_t rule_count,
                     const functor_t& functor) {

           const bool should_multithread =
                   Multithreading::should_multithread_rule_application(mt_policy, input.ElementCount, rule_count);

           typename SquareMatrix<output_elem>::StorageType output;
           if (should_multithread) {
               output.assign(input.dimension*input.dimension, output_elem{});
               Multithreading::transform_matrix_data(input.dimension, input.raw(), output.data(), functor);
           } else {
               // ST reduction:
               output.reserve(input.dimension * input.dimension);
               for (const auto& expr : input) {
                   output.emplace_back(functor(expr));
               }
           }
            return std::make_unique<SquareMatrix<output_elem>>(input.dimension, std::move(output));
        }
    }

    std::string SubstitutedMatrix::make_name() {
        std::stringstream ss;
        ss << "Substituted Matrix [Source: " << this->source_matrix.Description() << "; Rules: " << rules.name() << "]";
        return ss.str();
    }

    MonomialSubstitutedMatrix::MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                                         const MonomialMatrix& the_source,
                                                         const Multithreading::MultiThreadPolicy mt_policy)
         : MonomialMatrix{the_source.context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                          MonomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix(), mt_policy),
                          the_source.Hermitian() && msrb.is_hermitian()},
           SubstitutedMatrix{the_source, msrb} {

        this->description = this->make_name();
    }

    std::unique_ptr<SquareMatrix<Monomial>>
    MonomialSubstitutedMatrix::reduce( const MomentRulebook& msrb,
                                       const SquareMatrix<Monomial>& matrix,
                                       const Multithreading::MultiThreadPolicy mt_policy) {
        return do_reduction<Monomial>(matrix, mt_policy, msrb.size(), [&msrb](const Monomial& expr) {
            return msrb.reduce_monomial(expr);
        });
    }


    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                                             const MonomialMatrix& the_source,
                                                             const Multithreading::MultiThreadPolicy mt_policy)
         : PolynomialMatrix{the_source.context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix(), mt_policy)},
           SubstitutedMatrix{the_source, msrb}  {
        this->description = this->make_name();
    }

    PolynomialSubstitutedMatrix::PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                                             const PolynomialMatrix& the_source,
                                                             const Multithreading::MultiThreadPolicy mt_policy)
         : PolynomialMatrix{the_source.context, assert_symbols(symbols, the_source), msrb.factory.zero_tolerance,
                            PolynomialSubstitutedMatrix::reduce(msrb, the_source.SymbolMatrix(), mt_policy)},
           SubstitutedMatrix{the_source, msrb} {
        this->description = this->make_name();
    }

    std::unique_ptr<SquareMatrix<Polynomial>>
    PolynomialSubstitutedMatrix::reduce( const MomentRulebook& msrb,
                                         const SquareMatrix<Polynomial>& matrix,
                                         Multithreading::MultiThreadPolicy mt_policy) {
        return do_reduction<Polynomial>(matrix, mt_policy, msrb.size(), [&msrb](const Polynomial& expr) {
            return msrb.reduce(expr);
        });
    }

    std::unique_ptr<SquareMatrix<Polynomial>>
    PolynomialSubstitutedMatrix::reduce( const MomentRulebook& msrb,
                                         const SquareMatrix<Monomial>& matrix,
                                         Multithreading::MultiThreadPolicy mt_policy) {

        return do_reduction<Polynomial>(matrix, mt_policy, msrb.size(), [&msrb](const Monomial& expr) {
            return msrb.reduce(expr);
        });
    }

}