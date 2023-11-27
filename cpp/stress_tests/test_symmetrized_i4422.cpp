/**
 * test_symmetrization.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "test_symmetrized_i4422.h"

#include "report_outcome.h"

#include "dictionary/dictionary.h"
#include "matrix_system/matrix_system.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/party.h"
#include "scenarios/derived/lu_map_core_processor.h"

#include <chrono>
#include <iostream>
#include <stdexcept>


namespace Moment::StressTests {

    SymmetrizedI4422::SymmetrizedI4422() {
        this->lms_ptr = std::make_unique<Locality::LocalityMatrixSystem>(
                std::make_unique<Locality::LocalityContext>(Locality::Party::MakeList(2, 4, 2)));
    }

    std::unique_ptr<Symmetrized::Group> SymmetrizedI4422::make_group() const {
        using namespace Moment::Symmetrized;

        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(this->make_z2_generator());
        auto group_elems = Group::dimino_generation(generators, 2);
        if (group_elems.size() != 2) {
            throw std::runtime_error{"Expected two group elements after Dimino generation."};
        }

        auto base_rep_ptr = std::make_unique<Representation>(1, std::move(group_elems));
        auto group_ptr = std::make_unique<Group>(this->lms_ptr->localityContext, std::move(base_rep_ptr));
        if (group_ptr->size != 2) {
            throw std::runtime_error{"Group did not contain two elements after construction."};
        }
        if (group_ptr->fundamental_dimension != 9) {
            throw std::runtime_error{"Group 'fundamental' dimension should be size 9."};
        }
        return group_ptr;
    }

    const Symmetrized::Representation&
    SymmetrizedI4422::make_representation(Symmetrized::Group& group, size_t mm_level) const {
        return group.create_representation(mm_level*2, Multithreading::MultiThreadPolicy::Never);
    }

    size_t SymmetrizedI4422::ensure_base_dictionary(size_t mm_level) {
        this->lms_ptr->generate_dictionary(2*mm_level);
        auto& word_list = this->lms_ptr->Context().dictionary().Level(2*mm_level);
        return word_list().size();
    }


    std::unique_ptr<Symmetrized::SymmetrizedMatrixSystem>
    SymmetrizedI4422::make_symmetrized_system(std::unique_ptr<Symmetrized::Group> group_ptr, size_t mm_level) const {
        // Now, create new matrix system with group
        std::shared_ptr<Locality::LocalityMatrixSystem> lms_copy_ptr = this->lms_ptr;



        return std::make_unique<Symmetrized::SymmetrizedMatrixSystem>(std::move(lms_copy_ptr), std::move(group_ptr),
                                                                      mm_level*2,
                                                                      std::make_unique<Derived::LUMapCoreProcessor>(),
                                                                      -1,  Multithreading::MultiThreadPolicy::Never);
    }


    Eigen::SparseMatrix<double> SymmetrizedI4422::make_z2_generator() const {
        std::vector<Eigen::Triplet<double>> triplet_list;
        triplet_list.emplace_back(0, 0, 1.0);
        triplet_list.emplace_back(5, 1, 1.0);
        triplet_list.emplace_back(6, 2, 1.0);
        triplet_list.emplace_back(7, 3, 1.0);
        triplet_list.emplace_back(8, 4, 1.0);
        triplet_list.emplace_back(1, 5, 1.0);
        triplet_list.emplace_back(2, 6, 1.0);
        triplet_list.emplace_back(3, 7, 1.0);
        triplet_list.emplace_back(4, 8, 1.0);
        Eigen::SparseMatrix<double> output(9, 9);
        output.setFromSortedTriplets(triplet_list.begin(), triplet_list.end());
        return output;
    }

}


int main() {
    using namespace Moment::StressTests;
    using namespace Moment::Symmetrized;

    const size_t max_level = Moment::debug_mode ? 4 : 5;

    std::cout << "Creating base scenario... " << std::endl;
    const auto before_lms = std::chrono::high_resolution_clock::now();
    SymmetrizedI4422 i4422{};
    const auto done_lms = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> lms_duration = done_lms - before_lms;
    std::cout << "... done in " << lms_duration << "." << std::endl;


    for (size_t mm_level = 1; mm_level <= max_level; ++mm_level) {
        std::cout << "---\nMoment matrix level = " << mm_level << std::endl;
        /* FUNDAMENTAL */
        std::cout << "Generating fundamental group...";
        std::cout.flush();
        const auto before_fund = std::chrono::high_resolution_clock::now();
        std::unique_ptr<Group> group_ptr;
        try {
            group_ptr = i4422.make_group();
            report_success(before_fund);
        }
        catch (const std::exception& e) {
            report_failure(before_fund, e);
            return -1;
        }

        /* BASE SYSTEM WORD LIST */
        std::cout << "Generating group base dictionary of word length " << (mm_level*2) << "...";
        std::cout.flush();
        size_t words = 0;
        const auto before_dict = std::chrono::high_resolution_clock::now();
        try {
            words = i4422.ensure_base_dictionary(mm_level);
            report_success(before_dict);
        }
        catch (const std::exception& e) {
            report_failure(before_dict, e);
            return -1;
        }
        std::cout << "\tTotal words: " << words << std::endl;


        /* HIGHER DIMENSION REPRESENTATION */
        std::cout << "Generating group representation for word length " << (mm_level*2) << "...";
        std::cout.flush();
        size_t rep_dim = 0;
        const auto before_rep = std::chrono::high_resolution_clock::now();
        try {
            const auto& gr = i4422.make_representation(*group_ptr, mm_level);
            rep_dim = gr.dimension;
            report_success(before_rep);
        }
        catch (const std::exception& e) {
            report_failure(before_rep, e);
            return -1;
        }
        std::cout << "\tDimension: " << rep_dim << std::endl;

        /* MATRIX SYSTEM */
        std::cout << "Generating symmetrized matrix system for word length " << (mm_level*2) << "...";
        std::cout.flush();
        const auto before_sms = std::chrono::high_resolution_clock::now();
        std::unique_ptr<SymmetrizedMatrixSystem> sms_ptr;
        try {
            sms_ptr = i4422.make_symmetrized_system(std::move(group_ptr), mm_level);
            report_success(before_sms);
        }
        catch (const std::exception& e) {
            report_failure(before_sms, e);
            return -1;
        }
    }

    return 0;
}