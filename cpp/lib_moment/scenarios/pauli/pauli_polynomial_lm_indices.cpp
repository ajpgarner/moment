/**
 * pauli_polynomial_lm_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_polynomial_lm_indices.h"
#include "pauli_matrix_system.h"

#include "matrix/polynomial_matrix.h"

#include <sstream>

namespace Moment::Pauli {
    PauliPolynomialLMFactory::PauliPolynomialLMFactory(MatrixSystem& system)
        : PauliPolynomialLMFactory{dynamic_cast<PauliMatrixSystem&>(system)}  {  }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PauliPolynomialLMFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                       const PauliPolynomialLMFactory::Index& index,
                                                       Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_nearest_neighbour_localizing_matrix(lock, index, mt_policy);
        PolynomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{matrixIndex, matrixRef};
    }

    void PauliPolynomialLMFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                        const PauliPolynomialLMFactory::Index& index,
                                                        ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_localizing_matrix(lock, index, offset, matrix);
    }

    std::string PauliPolynomialLMFactory::not_found_msg(const PauliPolynomialLMFactory::Index& pmi) const {
        std::stringstream errSS;
        ContextualOS cErrSS{errSS, system.Context(), system.Symbols()};
        cErrSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cErrSS.format_info.show_braces = false;

        cErrSS << "Localizing matrix of Level " << pmi.Level.moment_matrix_level;
        if (pmi.Level.neighbours > 0 ) {
            errSS << " restricted to " << pmi.Level.neighbours
                  << " nearest neighbour" << ((pmi.Level.neighbours != 1) ? "s" : "");
        }
        cErrSS << " for polynomial \"" << pmi.Polynomial
               << "\" has not yet been generated.";

        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> PauliPolynomialLMFactory::get_write_lock() {
        return this->system.get_write_lock();
    }



}