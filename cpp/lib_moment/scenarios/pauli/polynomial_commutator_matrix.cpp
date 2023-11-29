/**
 * polynomial_commutator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_commutator_matrix.h"

#include "pauli_matrix_system.h"
#include "pauli_context.h"

#include <sstream>

namespace Moment::Pauli {
    namespace {
        template<bool anticommutator>
        std::string make_cm_description(const PauliContext& context, const SymbolTable& symbols,
                                        const PolynomialCommutatorMatrixIndex& index) {

            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << " Pauli ";
            if constexpr(anticommutator) {
                cSS << "Anti-Commutator";
            } else {
                cSS << "Commutator";
            }
            cSS << " Matrix, Level " << index.Level.moment_matrix_level << ",";
            if (index.Level.neighbours != 0) {
                cSS << index.Level.neighbours << " Neighbour";
                if (index.Level.neighbours != 1) {
                    cSS << "s";
                }
            }
            cSS << ", Phrase " << index.Polynomial;
            return ss.str();
        }
    }

    PolynomialCommutatorMatrix::PolynomialCommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                                           const PolynomialFactory& factory,
                                                           PolynomialCommutatorMatrixIndex index_in,
                                                           CompositeMatrix::ConstituentInfo&& constituents)
       : CompositeMatrix{context, symbols, factory, std::move(constituents)}, pauli_context{context},
         index{std::move(index_in)} {
        this->description = make_cm_description<false>(this->pauli_context, symbols, this->index);
    }

    PolynomialAnticommutatorMatrix::PolynomialAnticommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                                                   const PolynomialFactory& factory,
                                                                   PolynomialCommutatorMatrixIndex index_in,
                                                                   CompositeMatrix::ConstituentInfo&& constituents)
        : CompositeMatrix{context, symbols, factory, std::move(constituents)},
           pauli_context{context}, index{std::move(index_in)} {
            this->description = make_cm_description<true>(this->pauli_context,  symbols, this->index);
    }

    PolynomialCommutatorMatrixFactory::PolynomialCommutatorMatrixFactory(MatrixSystem& system)
        : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialCommutatorMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                       const PolynomialCommutatorMatrixFactory::Index& index,
                                                       Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_commutator_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{offset, matrix};
    }

    void PolynomialCommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                        const PolynomialCommutatorMatrixFactory::Index& index,
                                                        ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_commutator_matrix(lock, index, offset, matrix);
    }

    std::string PolynomialCommutatorMatrixFactory::not_found_msg(
            const PolynomialCommutatorMatrixFactory::Index& pmi) const {
        std::stringstream errSS;
        ContextualOS cErrSS{errSS, system.Context(), system.Symbols()};
        cErrSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cErrSS.format_info.show_braces = false;

        cErrSS << "Commutator matrix of Level " << pmi.Level.moment_matrix_level;
        if (pmi.Level.neighbours > 0 ) {
            errSS << " restricted to " << pmi.Level.neighbours
                  << " nearest neighbour" << ((pmi.Level.neighbours != 1) ? "s" : "");
        }
        cErrSS << " for polynomial \"" << pmi.Polynomial
               << "\" has not yet been generated.";

        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> PolynomialCommutatorMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }
    
    PolynomialAnticommutatorMatrixFactory::PolynomialAnticommutatorMatrixFactory(MatrixSystem& system)
        : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialAnticommutatorMatrixFactory::operator()(MaintainsMutex::WriteLock& lock,
                                                       const PolynomialAnticommutatorMatrixFactory::Index& index,
                                                       Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_anticommutator_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{offset, matrix};
    }

    void PolynomialAnticommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                        const PolynomialAnticommutatorMatrixFactory::Index& index,
                                                        ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_anticommutator_matrix(lock, index, offset, matrix);
    }

    std::string PolynomialAnticommutatorMatrixFactory::not_found_msg(
            const PolynomialAnticommutatorMatrixFactory::Index& pmi) const {
        std::stringstream errSS;
        ContextualOS cErrSS{errSS, system.Context(), system.Symbols()};
        cErrSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cErrSS.format_info.show_braces = false;

        cErrSS << "Anticommutator matrix of Level " << pmi.Level.moment_matrix_level;
        if (pmi.Level.neighbours > 0 ) {
            errSS << " restricted to " << pmi.Level.neighbours
                  << " nearest neighbour" << ((pmi.Level.neighbours != 1) ? "s" : "");
        }
        cErrSS << " for polynomial \"" << pmi.Polynomial
               << "\" has not yet been generated.";

        return errSS.str();
    }

    std::unique_lock<std::shared_mutex> PolynomialAnticommutatorMatrixFactory::get_write_lock() {
        return this->system.get_write_lock();
    }

}