/**
 * collins_gisin.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

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

        [[nodiscard]] OperatorSequence cgi_to_op_seq(const Context& context,
                                                     const std::vector<CollinsGisin::DimensionInfo>& dimInfo,
                                                     const CollinsGisinIndexView index) {
            sequence_storage_t ops;
            for (size_t d = 0, dMax = dimInfo.size(); d < dMax; ++d) {
                if (0 == index[d]) {
                    continue;
                }
                ops.emplace_back(dimInfo[d].op_ids[index[d]]);
            }
            return OperatorSequence{std::move(ops), context};
        }


    }


    CollinsGisinEntry::CollinsGisinEntry(const CollinsGisin& cgt, const CollinsGisinIndexView index)
        : sequence{cgi_to_op_seq(cgt.context, cgt.dimensionInfo, index)} {

    }

    CollinsGisin::CollinsGisin(const Context& context, const SymbolTable& symbol_table,
                               std::vector<size_t>&& dimensions, TensorStorageType storage)
            : AutoStorageTensor{std::move(dimensions), storage},
                    context{context}, symbol_table{symbol_table},
                    dimensionInfo(DimensionCount, DimensionInfo{}) {

        if (this->StorageType == TensorStorageType::Explicit) {
        }
    }

    void CollinsGisin::do_initial_symbol_search() {
        // Do nothing if in virtual-storage mode.
        if (this->StorageType == TensorStorageType::Virtual) {
            return;
        }

        // Do initial symbol search
        size_t index = 0;
        for (auto& datum : this->data) {
            auto * us = this->symbol_table.where(datum.sequence);
            if (us != nullptr) {
                assert(us->is_hermitian());
                assert(us->basis_key().second < 0);
                datum.symbol_id = us->Id();
                datum.real_index = us->basis_key().first;
            } else {
                this->missing_symbols.insert(this->missing_symbols.cend(), index);
            }
            ++index;
        }
    }


    bool CollinsGisin::fill_missing_symbols() noexcept {
        // Do nothing if in virtual-storage mode.
        if (this->StorageType == TensorStorageType::Virtual) {
            return true;
        }

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
            auto& datum = this->data[index];
            auto * us = symbol_table.where(datum.sequence);
            if (us != nullptr) {
                assert(us->is_hermitian());
                assert(us->basis_key().second < 0);
                datum.symbol_id = us->Id();
                datum.real_index = us->basis_key().first;
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

    OperatorSequence CollinsGisin::Sequence(const  CollinsGisinIndexView index) const {
        if (this->StorageType == TensorStorageType::Virtual) {
            this->validate_index(index);
            return cgi_to_op_seq(this->context, this->dimensionInfo, index);
        } else {
            return this->data[this->index_to_offset(index)].sequence;
        }
    }

    symbol_name_t CollinsGisin::Symbol(const CollinsGisinIndexView index) const {
        if (this->StorageType == TensorStorageType::Virtual) {
            this->validate_index(index);
            auto entry = this->make_element_no_checks(index);
            auto * us = this->symbol_table.where(entry.sequence);
            if (us != nullptr) {
                assert(us->is_hermitian());
                return us->Id();
            } else {
                throw make_missing_index_err(index, entry.sequence);
            }
        } else {
            const size_t offset = this->index_to_offset(index);

            std::shared_lock read_lock{this->symbol_mutex};
            if (this->missing_symbols.contains(offset)) {
                throw make_missing_index_err(index, this->data[offset].sequence);
            }

            return this->data[offset].symbol_id;
        }
    }

    ptrdiff_t CollinsGisin::RealIndex(const CollinsGisinIndexView index) const {
        if (this->StorageType == TensorStorageType::Virtual) {

            this->validate_index(index);
            auto entry = this->make_element_no_checks(index);
            auto * us = this->symbol_table.where(entry.sequence);
            if (us != nullptr) {
                assert(us->is_hermitian());
                assert(us->basis_key().second < 0);
                return us->basis_key().first;
            } else {
                throw make_missing_index_err(index, entry.sequence);
            }

        } else {
            const size_t offset = this->index_to_offset(index);

            std::shared_lock read_lock{this->symbol_mutex};
            if (this->missing_symbols.contains(offset)) {
                throw make_missing_index_err(index, this->data[offset].sequence);
            }

            return this->data[offset].real_index;
        }
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



    CollinsGisinEntry CollinsGisin::make_element_no_checks(Tensor::IndexView index) const {
        return Element{*this, index};
    }

}
