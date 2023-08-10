/**
 * moment_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"
#include "operator_matrix_factory.h"

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include "scenarios/context.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {

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

        if (context.can_have_aliases()) {
            const auto alias_mm_functor = [&context](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return context.simplify_as_moment(lhs * rhs);
            };
            OperatorMatrixFactory<MomentMatrix, decltype(alias_mm_functor)>
                    creation_context{context, symbols, level, alias_mm_functor, true, mt_policy};

            return creation_context.execute(level);
        } else {
            const auto mm_functor = [](const OperatorSequence &lhs, const OperatorSequence &rhs) {
                return lhs * rhs;
            };
            OperatorMatrixFactory<MomentMatrix, decltype(mm_functor)>
                    creation_context{context, symbols, level, mm_functor, true, mt_policy};

           return creation_context.execute(level);
        }
    }

}