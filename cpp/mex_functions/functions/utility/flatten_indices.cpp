/**
 * flatten_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "flatten_indices.h"

#include "utilities/index_flattener.h"

#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"

#include <algorithm>

namespace Moment::mex::functions {

    namespace {
        matlab::data::TypedArray<double> exportDoubleIndices(const IndexFlattener& flattener, bool zero_index) {
            matlab::data::ArrayFactory factory;
            auto double_output = factory.createArray<double>({1, flattener.size()});
            auto write_iter = double_output.begin();
            if (zero_index) {
                std::transform(flattener.begin(), flattener.end(), write_iter, [](const size_t index) {
                    return static_cast<double>(index);
                });
            } else {
                std::transform(flattener.begin(), flattener.end(), write_iter, [](const size_t index) {
                    return static_cast<double>(index + 1);
                });
            }
            return double_output;
        }
    }

    FlattenIndicesParams::FlattenIndicesParams(SortedInputs &&input) : SortedInputs(std::move(input)) {
        // Zero index mode?
        this->zero_index = this->flags.contains(u"zero_index");

        // Read object dimensions:
        this->dimensions = read_as_uint64_vector(matlabEngine, this->inputs[0]);


        // Read indices
        if (this->inputs[1].getType() == matlab::data::ArrayType::CELL) {
            matlab::data::CellArray cell_input = this->inputs[1];
            this->indices.reserve(cell_input.getNumberOfElements());
            for (auto iter = cell_input.cbegin(); iter != cell_input.cend(); ++iter) {
                this->indices.emplace_back(read_as_uint64_vector(matlabEngine, *iter));
            }
        } else {
            this->indices.emplace_back(read_as_uint64_vector(matlabEngine, this->inputs[1]));
        }

        const size_t index_count = this->indices.size();
        if (index_count > this->dimensions.size()) {
            throw_error(matlabEngine, errors::bad_param, "Cannot specify more index arrays than object dimensions.");
        }

        // Final index can act as a partial offset
        const size_t final_dim_size = std::reduce(this->dimensions.begin() + (this->indices.size()-1),
                                                  this->dimensions.end(),
                                                  static_cast<size_t>(1), std::multiplies{});


        if (this->zero_index) {
            // Zero-indexed input/output, so just bounds check.
            for (size_t dim = 0; dim < index_count; ++dim) {
                const auto &dimIndices = this->indices[dim];
                for (const auto x: dimIndices) {
                    const size_t max_val = ((dim + 1) < index_count) ? this->dimensions[dim] : final_dim_size;
                    if  (x >= max_val) {
                        std::stringstream errSS;
                        errSS << "Index '" << x << "' in dimension " << (dim+1) << " is out of range";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }
                }
            }
        } else {
            // One-indexing input/output, but we have to translate vectors to zero index for processing.
            for (size_t dim = 0; dim < index_count; ++dim) {
                auto &dimIndices = this->indices[dim];
                for (auto &x: dimIndices) {
                    const size_t max_val = ((dim + 1) < index_count) ? this->dimensions[dim] : final_dim_size;
                    if ((x < 1) || (x > max_val)) {
                        std::stringstream errSS;
                        errSS << "Index '" << x << "' in dimension " << (dim+1) << " is out of range";
                        throw_error(matlabEngine, errors::bad_param, errSS.str());
                    }
                    x -= 1;
                }
            }
        }

    }

    FlattenIndices::FlattenIndices(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"zero_index");

        this->min_inputs = 2;
        this->max_inputs = 2;
    }


    void FlattenIndices::operator()(IOArgumentRange output, FlattenIndicesParams &input) {
        IndexFlattener flattener{std::move(input.dimensions), std::move(input.indices)};
        output[0] = exportDoubleIndices(flattener, input.zero_index);
    }
}