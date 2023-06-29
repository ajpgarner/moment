/**
 * collins_gisin.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

#include "locality_context.h"

#include "matrix_system.h"
#include "symbolic/symbol_table.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment::Locality {
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

        [[nodiscard]] errors::BadCGError make_missing_err(const std::set<size_t>& missing_symbols,
                                                          const std::vector<OperatorSequence>& sequences) {
            std::stringstream errSS;
            errSS << "Not all symbol IDs for CG tensor could be found.";
            errSS << "\nMissing symbols for: ";
            bool once = false;
            for (auto opIndex : missing_symbols) {
                if (once) {
                    errSS << ", ";
                }
                errSS << sequences[opIndex].formatted_string();
                once = true;
            }
            return errors::BadCGError(errSS.str());
        }

    }

    CollinsGisin::CollinsGisin(const MatrixSystem &matrixSystem)
        : context{dynamic_cast<const LocalityContext&>(matrixSystem.Context())},
          Dimensions(make_dimensions(context.operators_per_party())) {

        const auto& symbol_table = matrixSystem.Symbols();

        const size_t total_size = get_total_size(this->Dimensions);

        this->sequences.reserve(total_size);
        this->symbols.reserve(total_size);
        this->real_indices.reserve(total_size);

        // Build array in column-major format, for quick export to matlab.
        for (const auto& cgIndex : MultiDimensionalIndexRange<true>{Dimensions}) {
            this->sequences.emplace_back(this->index_to_sequence(cgIndex));
        }

        // Do initial symbol search
        size_t index = 0;
        for (const auto& seq : this->sequences) {
            auto * us = symbol_table.where(seq);
            if (us != nullptr) {
                assert(us->is_hermitian());
                assert(us->basis_key().second < 0);
                this->symbols.emplace_back(us->Id());
                this->real_indices.emplace_back(us->basis_key().first);
            } else {
                this->symbols.emplace_back(-1);
                this->real_indices.emplace_back(-1);
                this->missing_symbols.insert(this->missing_symbols.cend(), index);
            }
            ++index;
        }
    }


    bool CollinsGisin::fill_missing_symbols(const SymbolTable& symbol_table) noexcept {
        // Early exit without lock if nothing missing
        std::shared_lock read_lock{this->symbol_mutex};
        if (this->missing_symbols.empty()) {
            return true;
        }
        read_lock.unlock();

        // Otherwise, try to acquire write lock
        std::unique_lock write_lock{this->symbol_mutex};

        // If symbols filled in interrim, then following code does nothing anyway:
        auto iter = this->missing_symbols.begin();
        while (iter != this->missing_symbols.end()) {
            size_t index = *iter;
            auto * us = symbol_table.where(this->sequences[index]);
            if (us != nullptr) {
                assert(us->is_hermitian());
                assert(us->basis_key().second < 0);
                this->symbols[index] = us->Id();
                this->real_indices[index] = us->basis_key().first;
                iter = this->missing_symbols.erase(iter);
            } else {
                ++iter;
            }
        }

        return this->missing_symbols.empty();
    }

    bool CollinsGisin::HasSymbols() const noexcept {
        std::shared_lock read_lock{this->symbol_mutex};
        return this->missing_symbols.empty();
    }

    const std::vector<symbol_name_t>& CollinsGisin::Symbols() const {
        // Safe lock, because if HasSymbols passes, then returned object will never change.
        std::shared_lock read_lock{this->symbol_mutex};

        if (!this->missing_symbols.empty()) {
            throw make_missing_err(this->missing_symbols, this->sequences);
        }
        return this->symbols;
    }

    const std::vector<symbol_name_t>& CollinsGisin::RealIndices() const {
        // Safe lock, because if HasSymbols passes, then returned object will never change.
        std::shared_lock read_lock{this->symbol_mutex};

        if (!this->missing_symbols.empty()) {
            throw make_missing_err(this->missing_symbols, this->sequences);
        }
        return this->real_indices;
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

        sequence_storage_t ops;
        for (size_t p = 0, pMax = context.Parties.size(); p < pMax; ++p) {
            if (0 == index[p]) {
                continue;
            }
            ops.emplace_back(context.Parties[p][index[p]-1]);
        }

        return OperatorSequence{std::move(ops), context};
    }


}
