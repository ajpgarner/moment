/**
 * full_correlator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "full_correlator.h"

#include "symbolic/polynomial_factory.h"
#include "utilities/combinations.h"

namespace Moment {

    namespace {

    }

    FullCorrelator::FullCorrelator(const CollinsGisin &collinsGisin, const PolynomialFactory &factory,
                                   TensorConstructInfo&& info, TensorStorageType storage)
       : PolynomialTensor(collinsGisin, factory, std::move(info.dimensions), storage),
         operator_offset(std::move(info.operator_offset)) {

        if (this->StorageType == TensorStorageType::Explicit) {
            this->hasAllSymbols = false;
            this->calculate_correlators();
        } else {
            this->hasAllSymbols = true;
        }
    }

    FullCorrelatorElement FullCorrelator::make_element_no_checks(AutoStorageIndexView index) const {
        // Get involved parties
        SmallVector<size_t, 8> involved_parties;
        for (size_t dim = 0; dim < this->DimensionCount; ++dim) {
            if (index[dim] > 0) {
                involved_parties.emplace_back(dim);
            }
        }

        // Switch appropriate maker
        switch (involved_parties.size()) {
            case 0:
                return make_id();
            case 1:
                return make_one_party(involved_parties[0], index);
            case 2:
                return make_two_party(involved_parties[0], involved_parties[1], index);
            default:
                return make_general(involved_parties, index);
        }
    }

    FullCorrelatorElement FullCorrelator::make_id() const {
        return FullCorrelatorElement {
            Polynomial::Scalar(1.0),
            Polynomial::Scalar(1.0)
        };
    }

    FullCorrelatorElement FullCorrelator::make_one_party(size_t party, FullCorrelator::IndexView index) const {
        const auto cg_offset = this->collinsGisin.index_to_offset_no_checks(index);
        auto cg_elem = this->collinsGisin.elem_no_checks(cg_offset);

        if (cg_elem->symbol_id > 0) {
            return FullCorrelatorElement{
                    {Monomial{static_cast<symbol_name_t>(cg_offset + 1), 2.0}, Monomial{1, -1.0}},
                    this->symbolPolynomialFactory({Monomial{cg_elem->symbol_id, 2.0}, Monomial{1, -1.0}})
            };
        } else {
            return FullCorrelatorElement{
                    {Monomial{static_cast<symbol_name_t>(cg_offset + 1), 2.0}, Monomial{1, -1.0}},
            };
        }
    }

    FullCorrelatorElement FullCorrelator::make_two_party(size_t partyA, size_t partyB,
                                                         FullCorrelator::IndexView ab_index) const {
        // Get offsets
        const auto ab_offset = this->collinsGisin.index_to_offset_no_checks(ab_index);
        const auto [a_offset, b_offset] = [&]() -> std::pair<size_t, size_t> { ;
            Index marginalIndex(this->DimensionCount, 0);
            marginalIndex[partyA] = ab_index[partyA];
            const auto a_off = this->collinsGisin.index_to_offset(marginalIndex);
            marginalIndex[partyA] = 0;
            marginalIndex[partyB] = ab_index[partyB];
            const auto b_off = this->collinsGisin.index_to_offset(marginalIndex);
            return std::make_pair(a_off, b_off);
        }();

        // Get elements
        const auto ab_elem = this->collinsGisin.elem_no_checks(ab_offset);
        const auto a_elem = this->collinsGisin.elem_no_checks(a_offset);
        const auto b_elem = this->collinsGisin.elem_no_checks(b_offset);

        const bool has_all = (ab_elem->symbol_id > 0) && (a_elem->symbol_id > 0) && (b_elem->symbol_id > 0);
        if (has_all) {
            return FullCorrelatorElement{
                    {Monomial{static_cast<symbol_name_t>(ab_offset + 1), 4.0},
                     Monomial{static_cast<symbol_name_t>(a_offset + 1), -2.0},
                     Monomial{static_cast<symbol_name_t>(b_offset + 1), -2.0},
                     Monomial{1, +1.0}},
                     this->symbolPolynomialFactory({Monomial{ab_elem->symbol_id, 4.0},
                                                    Monomial{a_elem->symbol_id, -2.0},
                                                    Monomial{b_elem->symbol_id, -2.0},
                                                    Monomial{1, +1.0}}),
            };
        } else {
            return FullCorrelatorElement{
                    {Monomial{static_cast<symbol_name_t>(ab_offset + 1), 4.0},
                            Monomial{static_cast<symbol_name_t>(a_offset + 1), -2.0},
                            Monomial{static_cast<symbol_name_t>(b_offset + 1), -2.0},
                            Monomial{1, +1.0}}
            };
        }
    }


    FullCorrelatorElement
    FullCorrelator::make_general(const SmallVector<size_t, 8> &involved_parties, std::span<const size_t> index) const {
        const size_t party_count = involved_parties.size();
        //
        // double factor = ((party_count % 2) == 0) ? 1.0 : -1.0;

        double factor = static_cast<double>(1 << party_count);

        // Prepare storage
        Polynomial::storage_t cgPolyData;
        cgPolyData.reserve(1 << party_count); // pascal's triangle / binomial expansion!

        // Full subset
        const auto final_cg_offset = this->index_to_offset_no_checks(index);
        cgPolyData.emplace_back(final_cg_offset+1, static_cast<double>(1 << party_count));
        factor *= -0.5;

        // Iterate through middle terms
        Index subsetIdx(this->DimensionCount, 0);

        for (size_t subset = party_count-1; subset > 0; --subset) {
            PartitionIterator partitions{party_count, subset};

            while (!partitions.done()) {
                std::fill(subsetIdx.begin(), subsetIdx.end(), 0);
                for (auto i = 0; i < subset; ++i) {
                    const auto party_idx = involved_parties[partitions.primary(i)];
                    subsetIdx[party_idx] = index[party_idx];
                }
                auto cgOffset = this->index_to_offset_no_checks(subsetIdx);
                cgPolyData.emplace_back(cgOffset+1, factor);
                ++partitions;
            }
            factor *= -0.5;
        }

        // Add constant term [0-elem subset]
        cgPolyData.emplace_back(1, factor);

        // Look up entries in CG tensor for symbolic polynomial
        FullCorrelatorElement elem{Polynomial(std::move(cgPolyData))};
        this->attempt_symbol_resolution(elem);

        return elem;
    }

    void FullCorrelator::calculate_correlators() {
        // Derive data
        this->data.reserve(this->ElementCount);
        MultiDimensionalIndexIterator<true> elementIndexIter(this->Dimensions);

        // Loop over elements
        this->hasAllSymbols = true;
        assert(this->missingSymbols.has_value());
        while (elementIndexIter) {
            const auto& elementIndex = *elementIndexIter;
            this->data.emplace_back(this->make_element_no_checks(elementIndex));
            if (!this->data.back().hasSymbolPoly) {
                this->missingSymbols->set(elementIndexIter.global());
                this->hasAllSymbols = false;
            }
            ++elementIndexIter;
        }
    }
}