/**
 * composite_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "composite_matrix.h"
#include "symbolic/polynomial_factory.h"

namespace Moment {

    namespace {

        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_empty_matrix(const size_t dimensions) {
            // Make a blank matrix
            return std::make_unique<PolynomialMatrix::MatrixData>(
                    dimensions, PolynomialMatrix::MatrixData::StorageType(dimensions * dimensions, Polynomial::Zero()));
        }

        template<typename elem_t>
        Polynomial scale_element(const PolynomialFactory& factory, const elem_t&, std::complex<double>);

        template<>
        Polynomial scale_element<Monomial>(const PolynomialFactory& factory,
                                           const Monomial& elem, std::complex<double> factor) {
            return Polynomial(Monomial{elem.id, elem.factor * factor, elem.conjugated}, factory.zero_tolerance);
        }

        template<>
        Polynomial scale_element<Polynomial>(const PolynomialFactory& factory,
                                             const Polynomial& elem, std::complex<double> factor) {
            auto output = elem;
            return elem * factor;
        }

        template<class matrix_t>
        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_rescaled_matrix(const PolynomialFactory& factory,
                             const matrix_t& input,
                             std::complex<double> factor) {

            const size_t dimension = input.Dimension();
            PolynomialMatrix::MatrixData::StorageType matrix_data;
            matrix_data.reserve(dimension * dimension);

            for (const auto& elem: input.SymbolMatrix()) {
                matrix_data.emplace_back(scale_element<typename matrix_t::ElementType>(factory, elem, factor));
            }
            for (auto& elem: matrix_data) {
                elem.fix_cc_in_place(factory.symbols, true, factory.zero_tolerance);
            }

            return std::make_unique<PolynomialMatrix::MatrixData>(dimension, std::move(matrix_data));
        }


        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_summed_matrix(const PolynomialFactory& factory, const CompositeMatrix::ConstituentInfo& constituents) {
            assert(constituents.size() > 1);

            // Get dimensions
            const size_t dimension = constituents.matrix_dimension;

            // General case: first divide constituents into monomial and polynomial parts.
            std::vector<std::pair<const Polynomial*, std::complex<double>>> polynomialParts;
            std::vector<std::pair<const Monomial*, std::complex<double>>> monomialParts;
            for (auto [matrixPtr, factor] : constituents.elements) {
                assert(matrixPtr);
                if (dimension != matrixPtr->Dimension()) {
                    throw std::logic_error{
                            "All constituent parts of polynomial localizing matrix should be same size!"};
                }

                if (matrixPtr->is_monomial()) {
                    auto mmPtr = dynamic_cast<const MonomialMatrix*>(matrixPtr);
                    assert(mmPtr);
                    const auto* monoDataPtr = mmPtr->raw_data();
                    monomialParts.emplace_back(monoDataPtr, factor);
                } else {
                    auto pmPtr = dynamic_cast<const PolynomialMatrix*>(matrixPtr);
                    assert(pmPtr);
                    const auto* polyDataPtr = pmPtr->raw_data();
                    polynomialParts.emplace_back(polyDataPtr, factor);
                }
            }

            // Make staging data
            const size_t numel = dimension * dimension;
            std::vector<Polynomial::storage_t> poly_data(numel);

            // Estimate memory requirements and pre-allocate
            const size_t num_constituents = constituents.size();
            for (size_t i = 0; i < numel; ++i) {
                poly_data.reserve(
                        num_constituents); // In practice, good estimate as most constituents will be monomial.
            }

            // Copy in data from monomials:
            for (const auto& [monoMatrix, factor]: monomialParts) {
                for (size_t i = 0; i < numel; ++i) {
                    const auto& monomial = monoMatrix[i];
                    poly_data[i].emplace_back(monomial.id, monomial.factor * factor, monomial.conjugated);
                }
            }

            // Copy in data from polynomials
            for (const auto& [polyMatrix, factor]: polynomialParts) {
                for (size_t i = 0; i < numel; ++i) {
                    for (const auto& monomial: polyMatrix[i]) {
                        poly_data[i].emplace_back(monomial.id, monomial.factor * factor, monomial.conjugated);
                    }
                }
            }

            // Finally, use factory to transform data into canonical form
            PolynomialMatrix::MatrixData::StorageType matrix_data;
            matrix_data.reserve(numel);
            for (auto& storage_data: poly_data) {
                matrix_data.emplace_back(factory(std::move(storage_data)));
            }

            // Construct:
            return std::make_unique<PolynomialMatrix::MatrixData>(dimension, std::move(matrix_data));
        };
    }


    CompositeMatrix::CompositeMatrix(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                                 CompositeMatrix::ConstituentInfo&& constituents)
         : PolynomialMatrix{context, symbols, factory.zero_tolerance,
                            CompositeMatrix::compile_to_polynomial_matrix_data(factory, constituents)},
           constituents{std::move(constituents)} {
    }

    std::unique_ptr<PolynomialMatrix::MatrixData>
    CompositeMatrix::compile_to_polynomial_matrix_data(const PolynomialFactory& factory,
                                                       const CompositeMatrix::ConstituentInfo& constituents) {
        // Special case: null matrix
        if (constituents.empty()) {
            return make_empty_matrix(constituents.matrix_dimension);
        }

        // Special case: single rescaled matrix
        if (constituents.size() == 1) {
            const auto& unique_element = constituents.elements[0];
            assert(unique_element.first);
            if (unique_element.first->is_monomial()) {
                return make_rescaled_matrix(factory, dynamic_cast<const MonomialMatrix&>(*unique_element.first),
                                            unique_element.second);
            } else {
                return make_rescaled_matrix(factory, dynamic_cast<const PolynomialMatrix&>(*unique_element.first),
                                            unique_element.second);
            }
        }

        // General case: have to sum matrices
        return make_summed_matrix(factory, constituents);
    }

}