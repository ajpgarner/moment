/**
 * polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_localizing_matrix.h"

#include "matrix/monomial_matrix.h"

#include "dictionary/dictionary.h"
#include "scenarios/context.h"
#include "symbolic/polynomial_factory.h"

namespace Moment {

    namespace {

        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_empty_matrix(const Context &context,
                          const PolynomialFactory& factory,
                          size_t level) {
            // We have to call for an OSG to infer dimensions
            const auto& osg_list = context.osg_list();
            const auto& osg = osg_list.Level(level)();
            const size_t dimensions = osg.size();

            // Then we can make a blank matrix
            PolynomialMatrix::MatrixData::StorageType empty_storage(dimensions*dimensions, Polynomial::Zero());
            return std::make_unique<PolynomialMatrix::MatrixData>(dimensions, std::move(empty_storage));
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
            //factory.
            auto output = elem;
            return elem * factor;
        }

        template<class matrix_t>
        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_scaled_matrix(const Context &context,
                          const PolynomialFactory& factory,
                          const matrix_t& input,
                          std::complex<double> factor) {

            const size_t dimension = input.Dimension();
            PolynomialMatrix::MatrixData::StorageType matrix_data;
            matrix_data.reserve(dimension*dimension);

            for (const auto& elem : input.SymbolMatrix()) {
                matrix_data.emplace_back(scale_element<typename matrix_t::ElementType>(factory, elem, factor));
            }
            for (auto &elem: matrix_data) {
                elem.fix_cc_in_place(factory.symbols, true, factory.zero_tolerance);
            }

            return std::make_unique<PolynomialMatrix::MatrixData>(dimension, std::move(matrix_data));
        }



        std::unique_ptr<PolynomialMatrix::MatrixData>
        make_summed_matrices(const Context &context, const PolynomialFactory& factory,
                     const PolynomialLMIndex& index,
                     const PolynomialLocalizingMatrix::Constituents& constituents) {
            assert(constituents.size() > 1);

            // Get dimensions
            const size_t dimension = constituents[0].first->Dimension();

            // General case: first divide constituents into monomial and polynomial parts.
            std::vector<std::pair<const Polynomial*, std::complex<double>>> polynomialParts;
            std::vector<std::pair<const Monomial*, std::complex<double>>> monomialParts;
            for (auto [matrixPtr, factor] : constituents) {
                assert(matrixPtr);
                if (dimension != matrixPtr->Dimension()) {
                    throw std::logic_error{"All constituent parts of polynomial localizing matrix should be same size!"};
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
            const size_t numel = dimension*dimension;
            std::vector<Polynomial::storage_t> poly_data(numel);

            // Estimate memory requirements and pre-allocate
            const size_t num_constituents = constituents.size();
            for (size_t i = 0; i < numel; ++i) {
                poly_data.reserve(num_constituents); // In practice, good estimate as most constituents will be monomial.
            }

            // Copy in data from monomials:
            for (const auto& [monoMatrix, factor] : monomialParts) {
                for (size_t i = 0; i < numel; ++i) {
                    const auto& monomial = monoMatrix[i];
                    poly_data[i].emplace_back(monomial.id, monomial.factor * factor, monomial.conjugated);
                }
            }

            // Copy in data from polynomials
            for (const auto& [polyMatrix, factor] : polynomialParts) {
                for (size_t i = 0; i < numel; ++i) {
                    for (const auto& monomial : polyMatrix[i]) {
                        poly_data[i].emplace_back(monomial.id, monomial.factor * factor, monomial.conjugated);
                    }
                }
            }

            // Finally, use factory to transform data into canonical form
            PolynomialMatrix::MatrixData::StorageType matrix_data;
            matrix_data.reserve(numel);
            for (auto& storage_data : poly_data) {
                matrix_data.emplace_back(factory(std::move(storage_data)));
            }

            // Construct:
            return std::make_unique<PolynomialMatrix::MatrixData>(dimension, std::move(matrix_data));
        };

        std::unique_ptr<PolynomialMatrix::MatrixData>
        sum_matrices(const Context &context, const PolynomialFactory& factory,
                     const PolynomialLMIndex& index,
                     const PolynomialLocalizingMatrix::Constituents& constituents) {
            // Special case: null matrix
            if (constituents.empty()) {
                return make_empty_matrix(context, factory, index.Level);
            }

            // Special case: one matrix rescaled
            if (constituents.size() == 1) {
                assert(constituents[0].first);
                if (constituents[0].first->is_monomial()) {
                    return make_scaled_matrix(context, factory,
                                              dynamic_cast<const MonomialMatrix&>(*constituents[0].first),
                                              constituents[0].second);
                } else {
                    return make_scaled_matrix(context, factory,
                                              dynamic_cast<const PolynomialMatrix&>(*constituents[0].first),
                                             constituents[0].second);
                }
            }

            // General case: have to sum matrices
            return make_summed_matrices(context, factory, index, constituents);
        }

        std::string make_description(const Context& context, const SymbolTable& symbols, const PolynomialLMIndex& index) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << "Localizing Matrix, Level " << index.Level
                << ", Phrase " << index.Polynomial;

            return ss.str();
        }
    }


    PolynomialLocalizingMatrix::PolynomialLocalizingMatrix(const Context &context, SymbolTable &symbols,
                                                           const PolynomialFactory& factory,
                                                           PolynomialLMIndex index,
                                                           PolynomialLocalizingMatrix::Constituents &&constituents)
           : PolynomialMatrix(context, symbols, factory.zero_tolerance,
                              sum_matrices(context, factory, index, constituents)),
                index{std::move(index)}, constituents{std::move(constituents)} {

        this->description = make_description(context, symbols, this->index);
    }

}