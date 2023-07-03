/**
 * collins_gisin.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"
#include "collins_gisin_iterator.h"

#include "symbolic/symbol_table.h"

#include <numeric>

namespace Moment {
    namespace {

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

        [[nodiscard]] errors::BadCGError make_missing_index_err(std::span<const size_t> index, const OperatorSequence& seq) {
            std::stringstream errSS;
            errSS << "The object at index [";
            bool once_cg = false;
            for (auto cgIndex : index) {
                if (once_cg) {
                    errSS << ", ";
                }
                errSS << cgIndex;
                once_cg = true;
            }
            errSS << "], corresponding to operator sequence \"" << seq << "\" does not yet exist in the symbol table.";

            return errors::BadCGError(errSS.str());
        }

        [[nodiscard]] inline size_t get_size(const std::vector<size_t>& elems) {
            return std::reduce(elems.cbegin(), elems.cend(), 1ULL, std::multiplies());
        }
    }

    CollinsGisin::CollinsGisin(std::vector<size_t>&& dimensions) : Tensor(std::move(dimensions)) {
        this->sequences.reserve(this->ElementCount);
        this->symbols.reserve(this->ElementCount);
        this->real_indices.reserve(this->ElementCount);
    }

    void CollinsGisin::do_initial_symbol_search(const SymbolTable& symbol_table) {
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

        // If symbols filled in interim, then following code does nothing anyway:
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

    bool CollinsGisin::HasAllSymbols() const noexcept {
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

    symbol_name_t CollinsGisin::Symbols(const CollinsGisinIndexView index) const {
        const size_t offset = this->index_to_offset(index);

        std::shared_lock read_lock{this->symbol_mutex};
        if (this->missing_symbols.contains(offset)) {
            throw make_missing_index_err(index, this->sequences[offset]);
        }

        return this->symbols[offset];
    }


    const std::vector<ptrdiff_t>& CollinsGisin::RealIndices() const {
        // Safe lock, because if HasSymbols passes, then returned object will never change.
        std::shared_lock read_lock{this->symbol_mutex};

        if (!this->missing_symbols.empty()) {
            throw make_missing_err(this->missing_symbols, this->sequences);
        }
        return this->real_indices;
    }

    ptrdiff_t CollinsGisin::RealIndices(const CollinsGisinIndexView index) const {
        const size_t offset = this->index_to_offset(index);

        std::shared_lock read_lock{this->symbol_mutex};
        if (this->missing_symbols.contains(offset)) {
            throw make_missing_index_err(index, this->sequences[offset]);
        }

        return this->real_indices[offset];
    }

    CollinsGisinIndex CollinsGisin::offset_to_index(size_t offset) const {
        if (offset > this->ElementCount) {
            throw errors::BadCGError("Offset out of bounds for CG table dimensions.");
        }

        const size_t dim_size = this->Dimensions.size();
        CollinsGisinIndex output;
        output.reserve(dim_size);
        for (size_t n = 0; n < dim_size; ++n) {
            output.emplace_back(offset % this->Dimensions.size());
            offset /= this->Dimensions.size();
        }
        return output;
    }


    CollinsGisinRange CollinsGisin::measurement_to_range(const std::span<const size_t> mmtIndices) const {
        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);

        for (auto mmtIndex : mmtIndices) {
            if (mmtIndex > this->gmIndex.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmInfo = this->gmIndex[mmtIndex];
            if (lower_bounds[gmInfo.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmInfo.party] = gmInfo.offset;
            upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

    CollinsGisinRange CollinsGisin::measurement_to_range(std::span<const size_t> mmtIndices,
                                                            std::span<const oper_name_t> fixedOutcomes) const {
        assert(mmtIndices.size() == fixedOutcomes.size());

        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);

        for (size_t n = 0; n < mmtIndices.size(); ++n) {

            if (mmtIndices[n] > this->gmIndex.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmInfo = this->gmIndex[mmtIndices[n]];
            if (lower_bounds[gmInfo.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            if (fixedOutcomes[n] >= 0) {
                lower_bounds[gmInfo.party] = gmInfo.offset + fixedOutcomes[n];
                upper_bounds[gmInfo.party] = gmInfo.offset + fixedOutcomes[n] + 1;
            } else {
                lower_bounds[gmInfo.party] = gmInfo.offset;
                upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
            }
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }


}
