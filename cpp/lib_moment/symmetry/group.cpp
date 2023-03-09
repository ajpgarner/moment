/**
 * group.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "group.h"

#include <iostream>

namespace Moment {

    namespace {
        Eigen::SparseMatrix<double> sparse_id_matrix(int dimension) {
            Eigen::SparseMatrix<double> id(dimension, dimension);
            id.setIdentity();
            return id;
        }
    }


    Group::Group(const std::vector<Eigen::SparseMatrix<double>>& generators) {
        auto elems = Group::dimino_generation(generators);

    }

    std::vector<Eigen::SparseMatrix<double>>
    Group::dimino_generation(const std::vector<Eigen::SparseMatrix<double>> &generators,
                                  const size_t max_subgroup_size) {
        std::vector<Eigen::SparseMatrix<double>> elements;

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
        Eigen::SparseMatrix<double> elem(*gen_iter);
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
                    const Eigen::SparseMatrix<double> next_coset_rep = (elements[rep_pos] * other_gen).pruned();

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