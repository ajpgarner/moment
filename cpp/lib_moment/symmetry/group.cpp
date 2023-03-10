/**
 * group.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "group.h"

#include "matrix/operator_sequence_generator.h"
#include "scenarios/context.h"

#include <iostream>

namespace Moment {

    namespace {
        repmat_t sparse_id_matrix(int dimension) {
            repmat_t id(dimension, dimension);
            id.setIdentity();
            return id;
        }
    }


    Group::Group(const Context& context, Representation&& rep)
        : context{context}, fundamental_dimension{rep.dimension} {

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

        // Save rep
        this->representations.emplace_back(std::move(rep));
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


}