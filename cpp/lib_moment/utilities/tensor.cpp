/**
 * tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "tensor.h"

#include <cassert>

#include <numeric>
#include <sstream>

namespace Moment {

    namespace {
        inline std::vector<size_t> make_strides(const std::vector<size_t>& dims) {
            if (dims.empty()) {
                return {};
            }

            std::vector<size_t> output;
            output.reserve(dims.size());
            output.push_back(1);
            std::partial_sum(dims.cbegin(), dims.cend()-1, std::back_inserter(output), std::multiplies());
            return output;
        }

        inline size_t take_prod(const std::vector<size_t>& dims, const std::vector<size_t>& strides) noexcept {
            if (dims.empty()) {
                return 0;
            }
            return strides.back() * dims.back();
        }
    }

    namespace errors {
        bad_tensor bad_tensor::no_data_stored(const std::string &name) {
            std::stringstream errSS;
            errSS << name << " has no explicitly stored elements.";
            return bad_tensor(errSS.str());
        }
    }

    Tensor::Tensor(std::vector<size_t> &&dimensions)
        : Dimensions(std::move(dimensions)), Strides(make_strides(Dimensions)),
          DimensionCount{this->Dimensions.size()}, ElementCount{take_prod(Dimensions, Strides)} {

    }

    void Tensor::validate_index(const Tensor::IndexView index) const {
        if (index.size() != this->Dimensions.size()) {
            std::stringstream errSS;
            errSS << "Index dimensions (" << index.size() << ") did not match "
                  << this->get_name(false) << " dimensions (" << this->DimensionCount << ").";
            throw errors::bad_tensor_index(errSS.str());
        }
        for (size_t n = 0; n < this->DimensionCount; ++n) {
            if (index[n] >= this->Dimensions[n]) {
                std::stringstream errSS;
                errSS << "Index '" << index[n] << "' for dimension " << n
                      << " of " << this->get_name(false) << " was out of bounds (maximum: "
                      << (this->Dimensions[n]-1) << ").";
                throw errors::bad_tensor_index(errSS.str());
            }
        }
    }

    void Tensor::validate_index_inclusive(const Tensor::IndexView index) const {
        if (index.size() != this->Dimensions.size()) {
            std::stringstream errSS;
            errSS << "Index dimensions (" << index.size() << ") did not match "
                  << this->get_name(false) << " dimensions (" << this->DimensionCount << ").";
            throw errors::bad_tensor_index(errSS.str());
        }
        for (size_t n = 0; n < this->DimensionCount; ++n) {
            if (index[n] > this->Dimensions[n]) {
                std::stringstream errSS;
                errSS << "Index '" << index[n] << "' for dimension " << n
                      << " of " << this->get_name(false) << " was out of bounds (maximum: "
                      << (this->Dimensions[n]) << ").";
                throw errors::bad_tensor_index(errSS.str());
            }
        }
    }

    void Tensor::validate_range(Tensor::IndexView min, Tensor::IndexView max) const {
        this->validate_index(min);
        this->validate_index_inclusive(max);
        for (size_t d= 0; d < this->DimensionCount; ++d) {
            if (min[d] > max[d]) {
                std::stringstream errSS;
                errSS << "Invalid splice dimension " << d << " of " << this->get_name(false) << ": "
                      << "Index " << min[d] << " must be smaller than index " << max[d] << ".";
                throw errors::bad_tensor_index(errSS.str());
            }
        }
    }

    void Tensor::validate_offset(const size_t offset) const {
        if (offset >= this->ElementCount) {
            std::stringstream errSS;
            errSS << "Offset " << offset << " was out of bounds (maximum: " << (this->ElementCount-1) << ").";
            throw errors::bad_tensor_index(errSS.str());
        }
    }

    size_t Tensor::index_to_offset_no_checks(const Tensor::IndexView index) const noexcept {
        return std::transform_reduce(this->Strides.cbegin(), this->Strides.cend(), index.begin(), 0ULL);
    }

    Tensor::Index Tensor::offset_to_index_no_checks(size_t offset) const {
        Index output;
        output.reserve(this->DimensionCount);
        for (size_t n = 0; n < this->DimensionCount; ++n) {
            output.emplace_back(offset % this->Dimensions[n]);
            offset /= this->Dimensions[n];
        }
        return output;
    }




}