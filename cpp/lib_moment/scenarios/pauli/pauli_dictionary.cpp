/**
 * pauli_dictionary.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_dictionary.h"
#include "pauli_context.h"
#include "pauli_osg.h"

#include <cassert>

namespace Moment::Pauli {

    PauliDictionary::PauliDictionary(const Moment::Pauli::PauliContext& context_in)
        : Dictionary{context_in}, pauliContext{context_in} { }

    const OSGPair& PauliDictionary::NearestNeighbour(const NearestNeighbourIndex& index) const {
        // If 0 neighbour mode, then default to normal OSG
        if (0 == index.neighbours) {
            return this->Level(index.moment_matrix_level);
        }

        // Otherwise, search Pauli OSG dictionary
        auto read_lock = this->get_read_lock();
        // Search for OSG
        auto find_key = this->nn_indices.find(index);
        if (find_key != this->nn_indices.end()) {
            const auto osg_offset = find_key->second;
            assert(osg_offset < this->osgs.size());
            return this->osgs[osg_offset]; // ~read_lock
        }
        read_lock.unlock();

        // OSG not found, so we create a new one
        auto new_osg = std::make_unique<PauliSequenceGenerator>(this->pauliContext, index);

        // Once generated, acquire write lock to store
        auto write_lock = const_cast<PauliDictionary*>(this)->get_write_lock();
        auto recheck_key = this->nn_indices.find(index);
        if (recheck_key != this->nn_indices.cend()) {
            // We were beaten in a race to create OSG, so discard work and return existing OSG.
            const auto osg_offset = find_key->second;
            assert(osg_offset < this->osgs.size());
            return this->osgs[osg_offset]; // ~write_lock, ~new_osg
        }

        // Do entry and index insertion
        const size_t insert_index = this->osgs.size();
        this->osgs.emplace_back(std::move(new_osg));
        assert(this->osgs.size() == (insert_index+1));
        this->nn_indices.insert(std::make_pair(index, insert_index));
        return this->osgs[insert_index]; // ~write_lock
    }
}