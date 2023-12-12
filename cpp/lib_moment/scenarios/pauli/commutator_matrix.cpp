/**
 * commutator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "commutator_matrix.h"
#include "pauli_matrix_system.h"

namespace Moment::Pauli {

    std::string MonomialCommutatorMatrix::description() const {
        const auto& nn_info = this->Index.Index;
        std::stringstream ss;
        ss << "Commutator matrix, level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->Index.Word;
        return ss.str();
    }

    std::string MonomialAnticommutatorMatrix::description() const {
        const auto& nn_info = this->Index.Index;
        std::stringstream ss;
        ss << "Anti-commutator matrix, level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->Index.Word;
        return ss.str();
    }

    MonomialCommutatorMatrixFactory::MonomialCommutatorMatrixFactory(MatrixSystem& system)
        : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, MonomialMatrix&>
    MonomialCommutatorMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                const MonomialCommutatorMatrixFactory::Index& index,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_commutator_matrix(lock, index, mt_policy);
        MonomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{matrixIndex, matrixRef};
    }

    void MonomialCommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                 const MonomialCommutatorMatrixFactory::Index& index,
                                                 const ptrdiff_t offset,
                                                 MonomialMatrix& matrix) {
        this->system.on_new_commutator_matrix(lock, index, offset, matrix);
    }

    std::string MonomialCommutatorMatrixFactory::not_found_msg(const MonomialCommutatorMatrixFactory::Index& pmi) const {
        std::stringstream errSS;
        errSS << "Could not find commutator matrix of level " << pmi.Index.moment_matrix_level;
        errSS << " for sequence \"" << this->system.Context().format_sequence(pmi.Word);
        if (pmi.Index.neighbours > 0 ) {
            errSS << ", restricted to " << pmi.Index.neighbours
                  << " nearest neighbour" << ((pmi.Index.neighbours != 1) ? "s" : "");
        }
        errSS << ".";
        return errSS.str();
    }

    MonomialAnticommutatorMatrixFactory::MonomialAnticommutatorMatrixFactory(MatrixSystem& system)
        : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, MonomialMatrix&>
    MonomialAnticommutatorMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                const MonomialAnticommutatorMatrixFactory::Index& index,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_anticommutator_matrix(lock, index, mt_policy);
        MonomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{matrixIndex, matrixRef};
    }

    void MonomialAnticommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                 const MonomialAnticommutatorMatrixFactory::Index& index,
                                                 const ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_anticommutator_matrix(lock, index, offset, matrix);
    }

    std::string
    MonomialAnticommutatorMatrixFactory::not_found_msg(const MonomialAnticommutatorMatrixFactory::Index& pmi) const {
        std::stringstream errSS;
        errSS << "Could not find anticommutator matrix of level " << pmi.Index.moment_matrix_level;
        errSS << " for sequence \"" << this->system.Context().format_sequence(pmi.Word);
        if (pmi.Index.neighbours > 0 ) {
            errSS << ", restricted to " << pmi.Index.neighbours
                  << " nearest neighbour" << ((pmi.Index.neighbours != 1) ? "s" : "");
        }
        errSS << ".";
        return errSS.str();
    }
}