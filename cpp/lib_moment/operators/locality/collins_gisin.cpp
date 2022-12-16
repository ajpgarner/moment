/**
 * collins_gisin.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "collins_gisin.h"

#include "locality_context.h"

#include "../matrix_system.h"
#include "../matrix/symbol_table.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment {
    namespace {
        [[nodiscard]] constexpr std::vector<size_t> make_dimensions(const std::vector<size_t>& oc) {
            std::vector<size_t> output;
            output.reserve(oc.size());
            for (auto val : oc) {
                output.push_back(val+1);
            }
            return output;
        }

        [[nodiscard]] constexpr size_t get_total_size(const std::vector<size_t>& dims) {
            size_t output = 1;
            for (auto val : dims) {
                output *= val;
            }
            return output;
        }

    }

    CollinsGisin::CollinsGisin(const MatrixSystem &matrixSystem)
        : context{dynamic_cast<const LocalityContext&>(matrixSystem.Context())},
          Dimensions(make_dimensions(context.operators_per_party())) {

        const auto& symbol_table = matrixSystem.Symbols();

        const size_t total_size = get_total_size(this->Dimensions);

        this->real_indices.reserve(total_size);
        this->symbols.reserve(total_size);
        this->sequences.reserve(total_size);

        // Build array in column-major format, for quick export to matlab.
        for (const auto& cgIndex : MultiDimensionalIndexRange<true>{Dimensions}) {
            this->sequences.emplace_back(this->index_to_sequence(cgIndex));
            auto * us = symbol_table.where(this->sequences.back());
            assert(us != nullptr);
            this->symbols.emplace_back(us->Id());
            this->real_indices.emplace_back(us->basis_key().first);
        }
    }

    void CollinsGisin::validate_index(const std::span<const size_t> index) const {
        if (index.size() != this->Dimensions.size()) {
            throw errors::BadCGError("Index dimensions must match CG table dimensions.");
        }
        for (size_t n = 0; n < index.size(); ++n) {
            if (index[n] >= this->Dimensions[n]) {
                throw errors::BadCGError("Index " + std::to_string(n) + " was out of bounds");
            }
        }
    }

    size_t CollinsGisin::index_to_offset(const std::span<const size_t> index) const {
        this->validate_index(index);

        size_t offset = 0;
        size_t stride = 1;
        for (size_t n = 0; n < index.size(); ++n) {
            offset += (index[n] * stride);
            stride *= Dimensions[n];
        }

        return offset;
    }

    OperatorSequence CollinsGisin::index_to_sequence(const std::span<const size_t> index) const {
        this->validate_index(index);

        std::vector<oper_name_t> ops;
        for (size_t p = 0, pMax = context.Parties.size(); p < pMax; ++p) {
            if (0 == index[p]) {
                continue;
            }
            ops.emplace_back(context.Parties[p][index[p]-1]);
        }

        return OperatorSequence(std::move(ops), context);
    }


}
