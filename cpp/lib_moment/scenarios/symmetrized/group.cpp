/**
 * group.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "group.h"
#include "representation_mapper.h"

#include "matrix/operator_sequence_generator.h"

#include "scenarios/context.h"

#include <atomic>
#include <iostream>


namespace Moment::Symmetrized {

    namespace {
        repmat_t sparse_id_matrix(int dimension) {
            repmat_t id(dimension, dimension);
            id.setIdentity();
            return id;
        }

        inline Representation * assertRP(std::unique_ptr<Representation>& rep_ptr) {
            if (!rep_ptr) {
                throw std::runtime_error{"Initial representation cannot be null pointer."};
            }
            return rep_ptr.get();
        }
    }


    Group::Group(const Context& context, std::unique_ptr<Representation> rep_ptr)
        : context{context}, fundamental_dimension{assertRP(rep_ptr)->dimension}, size{assertRP(rep_ptr)->size()} {

        // Push fundamental representation
        this->representations.emplace_back(std::move(rep_ptr));
        auto& rep = *(this->representations.front());

        // Calculate expected fundamental representation size:
        OperatorSequenceGenerator osg{context, 0, 1};
        const auto expected_size = osg.size();

        // Throw exception is unexpected size
        if (rep.dimension != expected_size) {
            std::stringstream errSS;
            errSS << "Initial representation has dimension " << rep.dimension
                  << ", but dimension " << expected_size
                  << " was expected (matching number of fundamental operators + 1).";
            throw std::runtime_error{errSS.str()};
        }
    }

    std::vector<repmat_t>
    Group::dimino_generation(const std::vector<repmat_t> &generators,
                                  const size_t max_subgroup_size) {
        std::vector<repmat_t> elements;

        // Special case of no generators, 1x1 identity only.
        if (generators.empty()) {
            elements.emplace_back(sparse_id_matrix(1));
            return elements;
        }

        // Otherwise, ascertain representation dimension, and insert ID element.
        auto gen_iter = generators.cbegin();
        assert(gen_iter != generators.cend());
        const int rep_dim = gen_iter->cols();
        assert(generators[0].rows() == rep_dim);
        const auto id = sparse_id_matrix(rep_dim);
        elements.emplace_back(id);

        // Generate orbit for first generator
        repmat_t elem(*gen_iter);
        size_t sg_index = 0;
        while ((sg_index < max_subgroup_size) && (!elem.isApprox(id))) {
            elements.emplace_back(elem);
            elem = (elem * (*gen_iter)).pruned();
            ++sg_index;
        }

        // Check first subgroup was actually generated
        if (!elem.isApprox(id)) {
            throw std::runtime_error{"Maximum subgroup size reached, but orbit of first generator was not completed."};
        }

        // Cycle over remaining generators
        ++gen_iter;
        for (;gen_iter != generators.cend(); ++gen_iter) {
            const auto& gen = *gen_iter;

            // Skip redundant generators
            if (std::any_of(elements.begin(), elements.end(),
                            [&gen](const auto& old_elem) { return gen.isApprox(old_elem); }) ) {
                continue;
            }

            // Apply generator to every element in set so far
            const size_t previous_order = elements.size();
            elements.reserve(previous_order*2);
            for (size_t prev_index = 0; prev_index < previous_order; ++prev_index) {
                const auto& prev_elem = elements[prev_index];
                elements.emplace_back((prev_elem * gen).pruned());
            }

            size_t rep_pos = previous_order;
            do {
                for (const auto& other_gen : generators) {
                    // Try to find a non-trivial new coset
                    const repmat_t next_coset_rep = (elements[rep_pos] * other_gen).pruned();

                    // Skip redundant coset
                    if (std::any_of(elements.begin(), elements.end(),
                                    [&next_coset_rep](const auto& old_elem) {
                            return next_coset_rep.isApprox(old_elem); }) ) {
                        continue;
                    }

                    // Add new coset
                    elements.reserve(elements.size() + previous_order);
                    for (size_t idx = 0; idx < previous_order; ++idx) {
                        elements.emplace_back((elements[idx] * next_coset_rep).pruned());
                    }
                }
                rep_pos += previous_order;
            } while (rep_pos < elements.size());
        }
        return elements;
    }

    const Representation& Group::create_representation(const size_t word_length) {
        if (word_length <= 0) {
            throw std::range_error{"Word length must be at least 1."};
        }
        const size_t index = word_length - 1;

        std::shared_lock read_lock{this->mutex};
        if ((index < this->representations.size()) && this->representations[index]) {
            return *this->representations[index]; // unlock read_lock
        }

        // Could not retrieve, obtain write lock to create
        read_lock.unlock();
        std::unique_lock write_lock{this->mutex};

        // Avoid race condition creation:~
        std::atomic_thread_fence(std::memory_order_acquire);
        if ((index < this->representations.size()) && this->representations[index]) {
            return *this->representations[index]; // unlock write_lock
        }

        // Create remapper
        RepresentationMapper remapper{this->context, word_length};

        // Remap group elements
        std::vector<repmat_t> remapped_elems;
        remapped_elems.reserve(this->size);
        const auto& fundamentals = this->representations[0]->group_elements();
        for (const auto& elem : fundamentals) {
            remapped_elems.emplace_back(remapper(elem));
        }

        // Construct new representation
        std::unique_ptr<Representation> rep = std::make_unique<Representation>(word_length, std::move(remapped_elems));

        // Do we need to expand list?
        if (this->representations.size() <= index) {
            this->representations.resize(index+1);
        }
        this->representations[index] = std::move(rep);

        std::atomic_thread_fence(std::memory_order_release);
        return *this->representations[index]; // Release write lock
    }

    const Representation& Group::representation(const size_t word_length) const {
        std::shared_lock lock{this->mutex};

        if (word_length <= 0) {
            throw std::range_error{"Word length must be at least 1."};
        }
        const size_t index = word_length - 1;

        if ((index  >= this->representations.size()) || (!this->representations[index ])) {
            std::stringstream errSS;
            errSS << "Representation of word length " << word_length << " has not yet been created.";
            throw std::runtime_error(errSS.str());
        }

        return *this->representations[index];
    }


}