/**
 * probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "probability_tensor.h"

#include "collins_gisin.h"

#include "scenarios/context.h"
#include "symbolic/polynomial_factory.h"

#include "utilities/combinations.h"

#include <numeric>

namespace Moment {

    namespace {

        size_t get_total_size(const std::vector<size_t>& dims) {
            return std::reduce(dims.cbegin(), dims.cend(), 1ULL, std::multiplies());
        }
    }

    ProbabilityTensor::ProbabilityTensor(const CollinsGisin &collinsGisin, const PolynomialFactory& factory,
                                         TensorConstructInfo&& info,
                                         const TensorStorageType storage)
        : PolynomialTensor{collinsGisin, factory, std::move(info.totalDimensions), storage} {

        this->make_dimension_info(info);

        if (this->StorageType == TensorStorageType::Explicit) {
            this->hasAllSymbols = false;
            this->calculate_implicit_symbols();
        } else {
            this->hasAllSymbols = true;
        }
    }

    void ProbabilityTensor::make_dimension_info(const TensorConstructInfo& info) {
        // Flag which indices have implicit symbols
        this->dimensionInfo.reserve(this->DimensionCount);

        // Build per-dimension information
        auto read_opm = info.outcomesPerMeasurement.cbegin();

        size_t global_mmt_id = 0;
        for (size_t d = 0; d < this->DimensionCount; ++d) {
            this->dimensionInfo.emplace_back(this->Dimensions[d]);
            auto& dimInfo = this->dimensionInfo.back();

            // First measurement for each party is ID, and it is always explicitly defined!
            dimInfo.outcome_index.emplace_back(0);

            dimInfo.cgDimensionIndex.emplace_back(0);
            size_t dim_index = 1;
            size_t cg_index = 1;

            // Now copy measurements
            for (size_t i = 0; i < info.mmtsPerParty[d]; ++i) {

                const auto outcomes = *read_opm;
                //std::fill_n(std::back_inserter(dimInfo.measurement), outcomes, global_mmt_id);
                const size_t first_cg_index = cg_index;
                for (size_t j = 0; j < outcomes-1; ++j) {
                    dimInfo.outcome_index.emplace_back(j);
                    dimInfo.cgDimensionIndex.emplace_back(cg_index);
                    ++cg_index;
                }

                dim_index += outcomes;
                dimInfo.outcome_index.emplace_back(outcomes - 1);
                if (info.fullyExplicit[global_mmt_id]) {
                    dimInfo.cgDimensionIndex.emplace_back(cg_index);
                    ++cg_index;
                } else {
                    dimInfo.cgDimensionIndex.emplace_back(first_cg_index);
                    dimInfo.implicit.set(dim_index-1);
                }

                ++read_opm;
                ++global_mmt_id;
            }
            assert(dim_index == this->Dimensions[d]);
        }
        assert(global_mmt_id == info.outcomesPerMeasurement.size());
        assert(read_opm == info.outcomesPerMeasurement.end());
    }

    void ProbabilityTensor::calculate_implicit_symbols() {
        assert(this->StorageType == TensorStorageType::Explicit);
        // Derive data
        this->data.reserve(this->ElementCount);
        MultiDimensionalIndexIterator<true> elementIndexIter(this->Dimensions);

        // Allocate outside of loop for speed...!
        ElementConstructInfo elemInfo(this->Dimensions.size());

        // Loop over elements
        this->hasAllSymbols = true;
        while (elementIndexIter) {
            const auto& elementIndex = *elementIndexIter;
            this->data.emplace_back(this->do_make_element(elementIndex, elemInfo));
            if (!this->data.back().hasSymbolPoly) {
                this->missingSymbols->set(elementIndexIter.global());
                this->hasAllSymbols = false;
            }
            ++elementIndexIter;
        }
    }

    std::vector<Polynomial> ProbabilityTensor::explicit_value_rules(const ProbabilityTensorRange &measurement,
                                                                    const std::span<const double> values) const {
        std::vector<Polynomial> output{};
        auto value_iter = values.begin();
        for (const ProbabilityTensorElement& elem : measurement) {
            assert(value_iter != values.end());
            // Each element must have symbols explicitly defined.
            if (!elem.hasSymbolPoly) {
                std::stringstream errSS;
                errSS << "Can not find symbols for polynomial \"";
                this->elem_as_string(errSS, elem);
                errSS << "\".";
                throw Moment::errors::BadPTError{errSS.str()};
            }
            output.emplace_back(elem.symbolPolynomial);
            this->symbolPolynomialFactory.append(output.back(), Polynomial::Scalar(-*value_iter));
            ++value_iter;
        }
        output.reserve(values.size());

        return output;
    }

    std::vector<Polynomial> ProbabilityTensor::explicit_value_rules(const ProbabilityTensorRange &measurement,
                                                                    const ProbabilityTensorElement &condition,
                                                                    const std::span<const double> values) const {
        // Check conditional element has symbols explicitly defined.
        if (!condition.hasSymbolPoly) {
            std::stringstream errSS;
            errSS << "Can not find symbols for polynomial \"";
            this->elem_as_string(errSS, condition);
            errSS << "\".";
            throw Moment::errors::BadPTError{errSS.str()};
        }

        std::vector<Polynomial> output{};
        output.reserve(values.size());
        auto value_iter = values.begin();
        for (const ProbabilityTensorElement& elem : measurement) {
            // Each element must have symbols explicitly defined.
            assert(value_iter != values.end());
            if (!elem.hasSymbolPoly) {
                std::stringstream errSS;
                errSS << "Can not find symbols for polynomial \"";
                this->elem_as_string(errSS, elem);
                errSS << "\".";
                throw Moment::errors::BadPTError{errSS.str()};
            }
            output.emplace_back(elem.symbolPolynomial);
            this->symbolPolynomialFactory.append(output.back(), condition.symbolPolynomial * -(*value_iter));

            ++value_iter;
        }

        return output;
    }

    ProbabilityTensor::ElementConstructInfo
    ProbabilityTensor::element_info(const ProbabilityTensorIndexView indices) const {
        this->validate_index(indices);
        ElementConstructInfo output(this->Dimensions.size());
        this->element_info(indices, output);
        return output;
    }

    void ProbabilityTensor::element_info(const ProbabilityTensorIndexView indices,
                                         ProbabilityTensor::ElementConstructInfo &output) const noexcept {
        output.implicitMmts.clear();
        for (size_t d = 0; d < this->Dimensions.size(); ++d) {
            const auto& dimInfo = this->dimensionInfo[d];
            const auto index = indices[d];
            output.baseIndex[d] = dimInfo.cgDimensionIndex[index];

            if (index > 0) {
                if (dimInfo.is_implicit(index)) {
                    // Index is implicit, so at some point will have to blit all implicit elements.
                    output.implicitMmts.emplace_back(d);
                    output.finalIndex[d] = output.baseIndex[d] + dimInfo.outcome_index[index]; // +1 - 1

                } else {
                    // Index not implicit, so will just need to copy one element.
                    output.finalIndex[d] = output.baseIndex[d] + 1;
                }
            } else {
                // Party not indexed, so is ID.
                output.finalIndex[d] = output.baseIndex[d] + 1;
            }
        }
    }

    ProbabilityTensorElement ProbabilityTensor::make_element_no_checks(const IndexView elementIndex) const {
        ElementConstructInfo elemInfo(this->Dimensions.size());
        return this->do_make_element(elementIndex, elemInfo);
    }

    ProbabilityTensorElement
    ProbabilityTensor::do_make_element(const IndexView elementIndex, ElementConstructInfo& elemInfo) const {

        // Get what measurement we refer to, and which indices are implied
        this->element_info(elementIndex, elemInfo);

        // Number of implicit indices to fill
        const auto num_implicit = elemInfo.implicitMmts.size();

        // Special case: no explicit variables, so construct single monomial
        if (num_implicit == 0) {

            const size_t cg_offset = this->collinsGisin.index_to_offset(elemInfo.baseIndex) + 1;
            const symbol_name_t symbol_id = this->collinsGisin.elem_no_checks(elemInfo.baseIndex)->symbol_id;
            if (symbol_id >= 0) {
                return ProbabilityTensorElement{Polynomial{Monomial{static_cast<symbol_name_t>(cg_offset), 1.0}},
                                                Polynomial{Monomial{symbol_id, 1.0}}};
            } else {
                return ProbabilityTensorElement{Polynomial{Monomial{static_cast<symbol_name_t>(cg_offset), 1.0}}};
            }

        }

        // Otherwise, we must build a polynomial algorithmically
        Polynomial::storage_t cg_poly_data;
        Polynomial::storage_t symbol_poly_data;
        bool symbol_poly_failed = false;

        // Parse through terms with alternating signs
        double the_sign = 1.0;
        for (size_t L = 0; L <= num_implicit; ++L) {



            // Special case 'normalization' term
            if (L == 0) {
                CollinsGisinIndex cgLookUp(elemInfo.baseIndex.cbegin(), elemInfo.baseIndex.cend());
                for (size_t rw_idx = 0; rw_idx < num_implicit; ++rw_idx) {
                     // Set to ID
                    const size_t remap_index = elemInfo.implicitMmts[rw_idx];
                    cgLookUp[remap_index] = 0;
                }

                const size_t cg_offset = this->collinsGisin.index_to_offset(cgLookUp) + 1;
                cg_poly_data.emplace_back(cg_offset, the_sign);

                const symbol_name_t symbol_id = this->collinsGisin.elem_no_checks(cgLookUp)->symbol_id;
                if (symbol_id >= 0) {
                    symbol_poly_data.emplace_back(symbol_id, the_sign);
                } else {
                    symbol_poly_failed = true;
                }

                the_sign = -the_sign;
                continue;
            }

            CollinsGisinIndex cgBase(elemInfo.baseIndex.cbegin(), elemInfo.baseIndex.cend());
            CollinsGisinIndex cgLast(elemInfo.finalIndex.cbegin(), elemInfo.finalIndex.cend());

            // Choose L free indices from the implicit indices
            PartitionIterator partitions{num_implicit, L};
            while (!partitions.done()) {
                // Get first and last CG elements
                for (size_t rw_idx = 0; rw_idx < num_implicit; ++rw_idx) {
                    size_t remap_index = elemInfo.implicitMmts[rw_idx];
                    if (partitions.bits(rw_idx)) { // Get all
                        cgBase[remap_index] = elemInfo.baseIndex[remap_index];
                        cgLast[remap_index] = elemInfo.finalIndex[remap_index];
                    } else { // Set to ID
                        cgBase[remap_index] = 0;
                        cgLast[remap_index] = 1;
                    }
                }

                CollinsGisin::CollinsGisinIterator cgIter{this->collinsGisin, CollinsGisinIndex(cgBase),
                                                                              CollinsGisinIndex(cgLast)};
                while (cgIter) {
                    cg_poly_data.emplace_back(cgIter.offset() + 1, the_sign); // Need offset to store ID as 1.

                    if (!symbol_poly_failed && (cgIter->symbol_id >= 0)) {
                        symbol_poly_data.emplace_back(cgIter->symbol_id, the_sign);
                    } else {
                        symbol_poly_failed = true;
                    }

                    ++cgIter;
                }
                ++partitions;
            }
            the_sign = -the_sign;
        }

        // Now, construct polynomial
        if (symbol_poly_failed) {
            return ProbabilityTensorElement{Polynomial{std::move(cg_poly_data)}};
        } else {
            return ProbabilityTensorElement{Polynomial{std::move(cg_poly_data)},
                                            this->symbolPolynomialFactory(std::move(symbol_poly_data))};
        }
    }



}