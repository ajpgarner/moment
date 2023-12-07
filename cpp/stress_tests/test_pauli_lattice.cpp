/**
 * test_pauli_lattice.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "test_pauli_lattice.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "report_outcome.h"


#include "integer_types.h"

#include <chrono>
#include <iostream>
#include <stdexcept>


namespace Moment::StressTests {
    PauliLattice::PauliLattice(size_t ch, size_t rw)
         : column_height{ch}, row_width{rw} {

    }

    Pauli::PauliMatrixSystem& PauliLattice::make_pms() {
        assert(!this->pms_ptr);
        using namespace Moment::Pauli;

        this->pms_ptr = std::make_unique<PauliMatrixSystem>(
                std::make_unique<PauliContext>(this->column_height, this->row_width, true, true)
        );
        if (!this->pms_ptr) {
            throw std::runtime_error{"PMS was not created...!"};
        }
        return *this->pms_ptr;
    }

    bool PauliLattice::test_moment_matrix(Pauli::NearestNeighbourIndex nni) {
        std::cout << "Generating moment matrix for level " << nni.moment_matrix_level;
        if (nni.neighbours != 0) {
            std::cout << " restricted to " << nni.neighbours << " nearest neighbour";
            if (nni.neighbours != 1) {
                std::cout << "s";
            }
        }
        std::cout << "...\n";
        std::cout.flush();

        const auto before_mm = std::chrono::high_resolution_clock::now();
        try {
            const auto& mm = this->pms_ptr->PauliMomentMatrices(nni);
            const auto done_mm = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double> mm_duration = done_mm - before_mm;
            std::cout << "\t... done in " << mm_duration
                       << " (size: " << mm.Dimension()
                       << ", symbols: " << this->pms_ptr->Symbols().size()
                       <<  ")." << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            report_failure(before_mm, e);
            return false;
        }
    }


}


int main() {
    using namespace Moment::StressTests;
    using namespace Moment::Pauli;

    const size_t max_lattice = Moment::debug_mode ? 8 : 16;
    for (size_t lattice = 2; lattice <= max_lattice; ++lattice) {
        std::cout << "LATTICE SIZE " << lattice << " x " << lattice << "\n";
        PauliLattice pl{lattice, lattice};

        // Make PMS and context
        std::cout << "Generating Pauli matrix system...";
        std::cout.flush();
        const auto before_pms = std::chrono::high_resolution_clock::now();
        try {
            pl.make_pms();
            report_success(before_pms);
        }
        catch (const std::exception& e) {
            report_failure(before_pms, e);
            return -1;
        }
        auto& pms = pl.pms();

        // Make MM level 1
        if (!pl.test_moment_matrix(NearestNeighbourIndex{1, 0})) {
            return -1;
        }

        // Make MM level 2, NN1
        if (!pl.test_moment_matrix(NearestNeighbourIndex{2, 1})) {
            return -1;
        }

        if (lattice <= (Moment::debug_mode ? 5 : 6)) {
            // Make MM level 2
            if (!pl.test_moment_matrix(NearestNeighbourIndex{2, 0})) {
                return -1;
            }
        }

//        if ((!Moment::debug_mode) && (lattice <= 6)) {
//            // Make MM level 3, NN1
//            if (!pl.test_moment_matrix(NearestNeighbourIndex{3, 1})) {
//                return -1;
//            }
//        }

        // Next...
        std::cout << "---\n";
    }

    return 0;
}