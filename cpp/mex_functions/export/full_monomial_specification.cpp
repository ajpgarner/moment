/**
 * full_monomial_specification.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "full_monomial_specification.h"


namespace Moment::mex {

    FullMonomialSpecification::FullMonomialSpecification(matlab::data::ArrayFactory &factory,
                                                         size_t length,
                                                         bool include_symbol_info)
            : dimensions({length, 1}), has_symbol_info{include_symbol_info},
              operators{factory.createCellArray(dimensions)},
              coefficients{factory.createArray<std::complex<double>>(dimensions)},
              hashes{factory.createArray<uint64_t>(dimensions)},
              symbol_ids{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                         : factory.createArray<int64_t>({0, 0})},
              is_conjugated{has_symbol_info ? factory.createArray<bool>(dimensions)
                                            : factory.createArray<bool>({0, 0})},
              real_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                               : factory.createArray<int64_t>({0, 0})},
              im_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                             : factory.createArray<int64_t>({0, 0})} {

    }


    FullMonomialSpecification::FullMonomialSpecification(matlab::data::ArrayFactory &factory,
                                                         matlab::data::ArrayDimensions array_dims,
                                                         bool include_symbol_info)
            : dimensions(std::move(array_dims)), has_symbol_info{include_symbol_info},
              operators{factory.createCellArray(dimensions)},
              coefficients{factory.createArray<std::complex<double>>(dimensions)},
              hashes{factory.createArray<uint64_t>(dimensions)},
              symbol_ids{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                         : factory.createArray<int64_t>({0, 0})},
              is_conjugated{has_symbol_info ? factory.createArray<bool>(dimensions)
                                            : factory.createArray<bool>({0, 0})},
              real_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                               : factory.createArray<int64_t>({0, 0})},
              im_basis_elems{has_symbol_info ? factory.createArray<int64_t>(dimensions)
                                             : factory.createArray<int64_t>({0, 0})} {

    }



    void FullMonomialSpecification::move_to_output(IOArgumentRange &output) noexcept {
        switch (output.size()) {
            default:
            case 7:
                output[6] = std::move(this->im_basis_elems);
            case 6:
                output[5] = std::move(this->real_basis_elems);
            case 5:
                output[4] = std::move(this->is_conjugated);
            case 4:
                output[3] = std::move(this->symbol_ids);
            case 3:
                output[2] = std::move(this->hashes);
            case 2:
                output[1] = std::move(this->coefficients);
            case 1:
                output[0] = std::move(this->operators);
            case 0:
                break;
        }
    }

    matlab::data::CellArray FullMonomialSpecification::move_to_cell(matlab::data::ArrayFactory &factory) {
        auto output = factory.createCellArray({1ULL, this->has_symbol_info ? 7ULL : 3ULL});
        output[0] = std::move(this->operators);
        output[1] = std::move(this->coefficients);
        output[2] = std::move(this->hashes);
        if (this->has_symbol_info) {
            output[3] = std::move(this->symbol_ids);
            output[4] = std::move(this->is_conjugated);
            output[5] = std::move(this->real_basis_elems);
            output[6] = std::move(this->im_basis_elems);
        }
        return output;
    }

    FullMonomialSpecification::partial_iter_t FullMonomialSpecification::partial_write_begin() {
        return partial_iter_t{this->operators.begin(), this->coefficients.begin(), this->hashes.begin()};
    }

    FullMonomialSpecification::partial_iter_t FullMonomialSpecification::partial_write_end() {
        return partial_iter_t{this->operators.end(), this->coefficients.end(), this->hashes.end()};
    }

    FullMonomialSpecification::full_iter_t FullMonomialSpecification::full_write_begin() {
        return full_iter_t{this->operators.begin(), this->coefficients.begin(), this->hashes.begin(),
                           this->symbol_ids.begin(), this->is_conjugated.begin(),
                           this->real_basis_elems.begin(), this->im_basis_elems.begin()};
    }

    FullMonomialSpecification::full_iter_t FullMonomialSpecification::full_write_end() {
        return full_iter_t{this->operators.end(), this->coefficients.end(), this->hashes.end(),
                           this->symbol_ids.end(), this->is_conjugated.end(),
                           this->real_basis_elems.end(), this->im_basis_elems.end()};
    }
}