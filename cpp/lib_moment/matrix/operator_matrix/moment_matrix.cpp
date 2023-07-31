/**
 * moment_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"
#include "operator_matrix_creation_context.h"

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include "scenarios/context.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {

    class MomentMatrixCreationContext : public OperatorMatrixCreationContext {
    public:
        MomentMatrixCreationContext(const Context &context, SymbolTable& symbols, size_t level,
                                        Multithreading::MultiThreadPolicy mt_policy)
                : OperatorMatrixCreationContext{context, symbols, level, mt_policy} {
        }

    protected:
        void make_operator_matrix_single_thread() override {
            this->operatorMatrix = do_make_operator_matrix_single_thread<MomentMatrix>(
                    [&](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                        return lhs * rhs;
                    },
                    this->Level
            );
            assert(this->operatorMatrix);
        }

        void make_operator_matrix_multi_thread() override {
            this->operatorMatrix = do_make_operator_matrix_multi_thread<MomentMatrix>(
                    [&](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                        return lhs * rhs;
                    },
                    this->Level
            );
            assert(this->operatorMatrix);
        }
    };

    MomentMatrix::MomentMatrix(const Context& context, const size_t level,
                               std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
        : OperatorMatrix{context, std::move(op_seq_mat)}, hierarchy_level{level} {

    }

    MomentMatrix::MomentMatrix(MomentMatrix &&src) noexcept :
            OperatorMatrix{static_cast<OperatorMatrix&&>(src)},
            hierarchy_level{src.hierarchy_level} {
    }

    MomentMatrix::~MomentMatrix() noexcept = default;

    const OperatorSequenceGenerator& MomentMatrix::Generators() const {
        return this->context.operator_sequence_generator(this->Level());
    }

    std::string MomentMatrix::description() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->hierarchy_level;
        return ss.str();
    }

    const MomentMatrix* MomentMatrix::as_monomial_moment_matrix_ptr(const SymbolicMatrix& input) noexcept {
        if (!input.is_monomial()) {
            return nullptr;
        }

        if (!input.has_operator_matrix()) {
            return nullptr;
        }

        const auto &op_matrix = input.operator_matrix();
        return dynamic_cast<const MomentMatrix *>(&op_matrix); // might be nullptr!
    }

    std::unique_ptr<MonomialMatrix>
    MomentMatrix::create_matrix(const Context &context, SymbolTable& symbols,
                                const size_t level, const Multithreading::MultiThreadPolicy mt_policy) {
        MomentMatrixCreationContext mmcc{context, symbols, level, mt_policy};
        mmcc.prepare_generators();
        mmcc.make_operator_matrix();
        mmcc.register_new_symbols();
        mmcc.make_symbolic_matrix();
        return mmcc.yield_matrix();
    }

}