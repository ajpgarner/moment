/**
 * tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "tensor.h"

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

    Tensor::Tensor(std::vector<size_t> &&dimensions)
        : Dimensions(std::move(dimensions)), Strides(make_strides(Dimensions)),
          DimensionCount{this->Dimensions.size()}, ElementCount{take_prod(Dimensions, Strides)} {
    }

    void Tensor::validate_index(const Tensor::IndexView index) const {
        if (index.size() != this->Dimensions.size()) {
            throw errors::bad_tensor_index("Index dimensions must match tensor dimensions.");
        }
        for (size_t n = 0; n < index.size(); ++n) {
            if (index[n] >= this->Dimensions[n]) {
                std::stringstream errSS;
                errSS << "Index '" << index[n] << "' for dimension " << n
                      << " was out of bounds (maximum: " << (this->Dimensions[n]-1) << ").";
                throw errors::bad_tensor_index(errSS.str());
            }
        }
    }

    size_t Tensor::index_to_offset_no_checks(const Tensor::IndexView index) const noexcept {
        return std::transform_reduce(this->Strides.cbegin(), this->Strides.cend(), index.begin(), 0ULL);
    }

}