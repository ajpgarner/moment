/**
 * probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "probability_tensor.h"

#include "collins_gisin.h"

#include "utilities/combinations.h"

#include <limits>
#include <numeric>

namespace Moment {

    namespace {

        size_t get_total_size(const std::vector<size_t>& dims) {
            return std::reduce(dims.cbegin(), dims.cend(), 1ULL, std::multiplies());
        }
    }

    ProbabilityTensor::ProbabilityTensor(const CollinsGisin &collinsGisin, ConstructInfo&& info,
                                         const TensorStorageType storage)
        : AutoStorageTensor{std::move(info.totalDimensions), storage},
          collinsGisin{collinsGisin}, hasSymbols(ElementCount) {

        this->make_dimension_info(info);

        this->calculate_implicit_symbols();


    }

    void ProbabilityTensor::make_dimension_info(const ConstructInfo& info) {
        // Flag which indices have implicit symbols
        this->dimensionInfo.reserve(this->DimensionCount);

        // Build per-dimension information
        auto read_opm = info.outcomesPerMeasurement.cbegin();
        size_t global_mmt_id = 1;
        for (size_t d = 0; d < this->DimensionCount; ++d) {
            this->dimensionInfo.emplace_back(this->Dimensions[d]);
            auto& dimInfo = this->dimensionInfo.back();

            // First measurement for each party is ID, and it is always explicitly defined!
            dimInfo.outcome_index.emplace_back(0);
            dimInfo.measurement.emplace_back(0);
            dimInfo.cgDimensionIndex.emplace_back(0);
            size_t dim_index = 1;
            size_t cg_index = 1;

            // Now copy remaining measurements
            for (size_t i = 0; i < info.mmtsPerParty[d]; ++i) {
                ++global_mmt_id;
                const auto outcomes = *read_opm;
                std::fill_n(std::back_inserter(dimInfo.measurement), outcomes, global_mmt_id);
                const size_t first_cg_index = cg_index;
                for (size_t j = 0; j < outcomes-1; ++j) {
                    dimInfo.outcome_index.emplace_back(j);
                    dimInfo.cgDimensionIndex.emplace_back(cg_index);
                    ++cg_index;
                }
                dimInfo.outcome_index.emplace_back(outcomes);
                dimInfo.cgDimensionIndex.emplace_back(first_cg_index);

                dim_index += outcomes;
                dimInfo.implicit.set(dim_index-1);
                ++read_opm;

            }
            assert(dim_index == this->Dimensions[d]);
        }
        assert(read_opm == info.outcomesPerMeasurement.end());
    }

    void ProbabilityTensor::calculate_implicit_symbols() {
        // Derive data
        this->data.reserve(this->ElementCount);
        MultiDimensionalIndexIterator<true> elementIndexIter(this->Dimensions);

        // Allocate outside of loop for speed...!
        ElementConstructInfo elemInfo(this->Dimensions.size());

        // Loop over elements
        while (elementIndexIter) {
            const auto& elementIndex = *elementIndexIter;
            this->data.emplace_back(this->do_make_element(elementIndex, elemInfo));
            ++elementIndexIter;
        }
    }


    Polynomial ProbabilityTensor::CGPolynomial(const ProbabilityTensorIndexView indices) const {
        this->validate_index(indices);
        if (this->StorageType == TensorStorageType::Explicit) {
            return this->data[this->index_to_offset_no_checks(indices)].cgPolynomial;
        } else {
            return this->make_element_no_checks(indices).cgPolynomial;
        }
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
                    output.finalIndex[d] = output.baseIndex[d] + dimInfo.outcome_index[index];

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

    ProbabilityTensorElement ProbabilityTensor::make_element_no_checks(const Tensor::IndexView elementIndex) const {
        ElementConstructInfo elemInfo(this->Dimensions.size());
        return this->do_make_element(elementIndex, elemInfo);
    }

    ProbabilityTensorElement
    ProbabilityTensor::do_make_element(const Tensor::IndexView elementIndex, ElementConstructInfo& elemInfo) const {

        // Get what measurement we refer to, and which indices are implied
        this->element_info(elementIndex, elemInfo);

        // Number of implicit indices to fill
        const auto num_implicit = elemInfo.implicitMmts.size();

        if (num_implicit == 0) {
            // Construct single monomial
            const auto index = this->collinsGisin.index_to_offset(elemInfo.baseIndex);

            return ProbabilityTensorElement{Polynomial{Monomial{static_cast<symbol_name_t>(index), 1.0}}};
        }

        // Otherwise, we must build a polynomial algorithmically
        Polynomial::storage_t symbolComboData;
        double the_sign = (num_implicit % 2 == 0) ? +1. : -1.;

        // L = 0 term is ID.
        for (size_t L = 1; L <= num_implicit; ++L) {

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

                // TODO: iterator 'repurpose'
                CollinsGisin::CollinsGisinIterator cgIter{this->collinsGisin, CollinsGisinIndex(cgBase),
                                                                              CollinsGisinIndex(cgLast)};
                while (cgIter) {
                    symbolComboData.emplace_back(cgIter.offset(), the_sign);
                    ++cgIter;
                }
                ++partitions;
            }
            the_sign = -the_sign;
        }

        // Finally, find the "Normalization" term
        assert(the_sign == 1); // If correctly alternating, normalization should be positive always.
        symbolComboData.emplace_back(1, the_sign);

        // Now, construct polynomial
        return ProbabilityTensorElement{Polynomial{std::move(symbolComboData)}};
    }

}