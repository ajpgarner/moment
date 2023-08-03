/**
 * extended_matrix_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "extended_matrix_indices.h"
#include "inflation_matrix_system.h"
#include "extended_matrix.h"

#include <sstream>

namespace Moment::Inflation {

    std::vector<symbol_name_t> ExtendedMatrixIndex::set_to_vector(const std::set<symbol_name_t> &input) {
        std::vector<symbol_name_t> output;
        output.reserve(input.size());
        std::copy(input.cbegin(), input.cend(), std::back_inserter(output));
        return output;
    }

    ExtendedMatrixFactory::ExtendedMatrixFactory(MatrixSystem &system)
        : ExtendedMatrixFactory{dynamic_cast<InflationMatrixSystem&>(system)} { }

    ptrdiff_t ExtendedMatrixIndexStorage::find(const Index& index) const noexcept {
        // Do we have root node?
        const auto* indexRoot = this->extension_indices.find_node(static_cast<symbol_name_t>(index.moment_matrix_level));
        if (nullptr == indexRoot) {
            return -1;
        }

        // Try to find extensions
        auto where = indexRoot->find(index.extension_list);
        if (!where.has_value()) {
            return -1;
        }
        return where.value();
    }

    bool ExtendedMatrixIndexStorage::contains(const Index& index) const noexcept {
        // Do we have root node?
        const auto* indexRoot = this->extension_indices.find_node(static_cast<symbol_name_t>(index.moment_matrix_level));
        if (nullptr == indexRoot) {
            return false;
        }

        // Try to find extensions
        auto where = indexRoot->find(index.extension_list);
        return where.has_value();
    }

    std::pair<ptrdiff_t, bool> ExtendedMatrixIndexStorage::insert(const Index& index, ptrdiff_t offset)  {
        // Create (or find) root node.
        auto * root = this->extension_indices.add_node(static_cast<symbol_name_t>(index.moment_matrix_level));
        // Do insert
        return root->add_if_new(index.extension_list, offset);
    }

    std::pair<ptrdiff_t, ExtendedMatrix &>
    ExtendedMatrixFactory::operator()(MaintainsMutex::WriteLock &lock, const ExtendedMatrixFactory::Index &index,
                                      Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto extended_matrix_ptr = this->system.createNewExtendedMatrix(lock, index, mt_policy);
        auto& matrix = *extended_matrix_ptr;
        this->system.push_back(std::move(extended_matrix_ptr));
        ptrdiff_t offset = static_cast<ptrdiff_t>(this->system.size()) - 1;
        return std::pair<ptrdiff_t, ExtendedMatrix &>{offset, matrix};
    }

    void ExtendedMatrixFactory::notify(const Index& index, ExtendedMatrix& matrix) {
        this->system.onNewExtendedMatrixCreated(index, matrix);
    }

    std::string ExtendedMatrixFactory::not_found_msg(const ExtendedMatrixFactory::Index &index) const {

        const bool has_mm = this->system.MomentMatrix.contains(index.moment_matrix_level);

        std::stringstream errSS;
        if (!has_mm) {
            errSS << "An extended matrix for moment matrix level " << index.moment_matrix_level << " was not found,"
                  << " because moment matrix level " << index.moment_matrix_level << " has not yet been generated.";
        } else {
            errSS << "Could not find extended matrix for moment matrix level " << index.moment_matrix_level << " ";
            if (index.extension_list.empty()) {
                errSS << "with no extensions.";
            } else {
                errSS << "extended by symbols ";
                const size_t num_ext = index.extension_list.size();
                const auto write_ext = std::min<size_t>(num_ext, 10ULL);
                bool once = false;
                for (size_t i = 0; i < write_ext; ++i) {
                    if (once) {
                        errSS << ", ";
                    }
                    errSS << "#" << index.extension_list[i];
                    once = true;
                }

                if (write_ext < num_ext) {
                    errSS << ", and " << (num_ext - write_ext) << " other symbols.";
                } else {
                    errSS << ".";
                }
            }
        }
        return errSS.str();
    }

    MaintainsMutex::WriteLock ExtendedMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }


}
