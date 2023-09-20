/**
 * test_inflated_triangle.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "test_inflated_triangle.h"

#include "integer_types.h"

#include "matrix/monomial_matrix.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/extension_suggester.h"
#include "scenarios/inflation/inflation_context.h"

#include <chrono>
#include <iostream>

namespace Moment::StressTests {
    using namespace Moment::Inflation;

    InflatedTriangle::InflatedTriangle(size_t outcomes, size_t inflation_level)
        : OutcomesPerCorner{outcomes}, InflationLevel{inflation_level} { }

    void InflatedTriangle::set_up_ims() {
        this->ims_ptr = std::make_unique<InflationMatrixSystem>(
                std::make_unique<InflationContext>(CausalNetwork{{OutcomesPerCorner, OutcomesPerCorner, OutcomesPerCorner},
                                                                 {{0, 1}, {1, 2}, {0, 2}}},
                                                   InflationLevel));
    }

    const SymbolicMatrix& InflatedTriangle::make_moment_matrix(const size_t mm_level) {
        return this->ims_ptr->MomentMatrix(mm_level);
    }

    const SymbolicMatrix& InflatedTriangle::make_extended_matrix(size_t mm_level) {
        const auto& mm = this->ims_ptr->MomentMatrix(mm_level);
        const auto& mm_as_mono = dynamic_cast<const MonomialMatrix&>(mm);

        auto extensions = this->ims_ptr->suggest_extensions(mm_as_mono);

        const auto& em = this->ims_ptr->ExtendedMatrices(ExtendedMatrixIndex{mm_level, std::move(extensions)});

        if ((mm.Dimension() + extensions.size()) != em.Dimension()) {
            throw std::runtime_error{"Extended matrix dimensions did not match expectations."};
        }

        return em;
    }

}

int main() {
    using namespace Moment::StressTests;

    const size_t num_outcomes = 4;
    const size_t max_inflation_level = !Moment::debug_mode ? 3 : 2;

    for (size_t inflation_level = 1; inflation_level <= max_inflation_level; ++inflation_level ) {
        std::cout << "---\nInflation level = " << inflation_level << std::endl;

        InflatedTriangle triangle{num_outcomes, inflation_level};
        std::cout << "Setting up matrix system..." << std::endl;
        const auto before_ams = std::chrono::high_resolution_clock::now();
        try {
            triangle.set_up_ims();

            const auto done_ams = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ams_duration = done_ams - before_ams;
            std::cout << "... done in " << ams_duration << "." << std::endl;
        } catch (const std::exception &e) {
            const auto failure_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ams_duration = failure_time - before_ams;
            std::cout << "... failed after " << ams_duration << ": " << e.what() << std::endl;
            return -1;
        }

        const size_t max_MM = (inflation_level <= 2) ? (!Moment::debug_mode ? 3 : 2) : 2;
        for (size_t mm_level = 1; mm_level <= max_MM; ++mm_level) {
            std::cout << "Generating moment matrix level " << mm_level << "..." << std::endl;
            const auto before_mm = std::chrono::high_resolution_clock::now();
            try {
                auto &mm = triangle.make_moment_matrix(mm_level);
                const auto done_mm = std::chrono::high_resolution_clock::now();
                const std::chrono::duration<double> mm_duration = done_mm - before_mm;
                std::cout << "... done in " << mm_duration << " (size: " << mm.Dimension() << ")." << std::endl;
            } catch (const std::exception &e) {
                const auto failure_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> mm_duration = failure_time - before_ams;
                std::cout << "... failed after " << mm_duration << ": " << e.what() << std::endl;
                return -1;
            }
            std::cout << "Generating extended matrix level " << mm_level << "..." << std::endl;
            const auto before_em = std::chrono::high_resolution_clock::now();
            try {
                auto &em = triangle.make_extended_matrix(mm_level);
                const auto done_em = std::chrono::high_resolution_clock::now();
                const std::chrono::duration<double> em_duration = done_em - before_em;
                std::cout << "... done in " << em_duration << "." << std::endl;
            } catch (const std::exception &e) {
                const auto failure_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> em_duration = failure_time - before_em;
                std::cout << "... failed after " << em_duration << ": " << e.what() << std::endl;
                return -1;
            }

        }
        std::cout << "\n";
    }
    return 0;
}