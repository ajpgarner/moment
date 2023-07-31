/**
 * operator_matrix_creation_context.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "operator_matrix_creation_context.h"

#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"

namespace Moment {

    OperatorMatrixCreationContext::OperatorMatrixCreationContext(const Context& context, SymbolTable& symbols,
                                                                 const size_t level,
                                                                 const Multithreading::MultiThreadPolicy mt_policy)
        : context{context}, symbols{symbols}, Level{level}, mt_policy{mt_policy} {

    }

    OperatorMatrixCreationContext::~OperatorMatrixCreationContext() noexcept = default;

    void OperatorMatrixCreationContext::prepare_generators() {
        // Make or get generators
        this->colGen = &context.operator_sequence_generator(Level);
        this->rowGen = &context.operator_sequence_generator(Level, true);

        // Build matrix...
        this->dimension = colGen->size();
        assert(this->dimension == rowGen->size());

        // Determine, from dimension, whether to go into MT mode.
        const bool use_multithreading
                = Multithreading::should_multithread_matrix_creation(mt_policy, this->dimension*this->dimension);
        if (use_multithreading) {
            this->mt_policy = Multithreading::MultiThreadPolicy::Always;
        } else {
            this->mt_policy = Multithreading::MultiThreadPolicy::Never;
        }
    }

    void OperatorMatrixCreationContext::make_operator_matrix() {
        if (this->Multithread()) {
            this->make_operator_matrix_multi_thread();
        } else {
            this->make_operator_matrix_single_thread();
        }
    }

    void OperatorMatrixCreationContext::register_new_symbols() {
        if (this->Multithread()) {
            this->register_new_symbols_multi_thread();
        } else {
            this->register_new_symbols_single_thread();
        }
    }

    void OperatorMatrixCreationContext::make_symbolic_matrix() {
        if (this->Multithread()) {
            this->make_symbolic_matrix_multi_thread();
        } else {
            this->make_symbolic_matrix_single_thread();
        }
    }

    std::unique_ptr<MonomialMatrix> OperatorMatrixCreationContext::yield_matrix() noexcept {
        assert(this->symbolicMatrix);
        return std::move(this->symbolicMatrix);
    }

    void OperatorMatrixCreationContext::register_new_symbols_single_thread() {

    }

    void OperatorMatrixCreationContext::register_new_symbols_multi_thread() {

    }

    void OperatorMatrixCreationContext::make_symbolic_matrix_single_thread() {
        // TODO: Parallelize
        this->symbolicMatrix = std::make_unique<MonomialMatrix>(symbols, std::move(this->operatorMatrix));
    }

    void OperatorMatrixCreationContext::make_symbolic_matrix_multi_thread() {
        // TODO: Parallelize
        this->symbolicMatrix = std::make_unique<MonomialMatrix>(symbols, std::move(this->operatorMatrix));
    }

}