/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"
#include "operator_matrix_factory.h"

#include "dictionary/operator_sequence_generator.h"

#include "scenarios/context.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include <sstream>

namespace Moment {
    namespace {
        inline const Context& assert_context(const Context& context, const LocalizingMatrixIndex& lmi) {
            assert(lmi.Word.is_same_context(context));
            return context;
        }
    }

    LocalizingMatrix::LocalizingMatrix(const Context& context, LocalizingMatrixIndex lmi,
                                       std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
        : OperatorMatrix{assert_context(context, lmi), std::move(op_seq_mat)}, Index{std::move(lmi)} {

    }

    LocalizingMatrix::~LocalizingMatrix() noexcept = default;

    std::string LocalizingMatrix::description() const {
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << this->Index.Level << ", Word " << this->Index.Word;
        return ss.str();
    }


    const LocalizingMatrix* LocalizingMatrix::as_monomial_localizing_matrix_ptr(const SymbolicMatrix& input) noexcept {
        if (!input.is_monomial()) {
            return nullptr;
        }

        if (!input.has_operator_matrix()) {
            return nullptr;
        }

        const auto& op_matrix = input.operator_matrix();
        return dynamic_cast<const LocalizingMatrix*>(&op_matrix); // might be nullptr!
    }

    std::unique_ptr<MonomialMatrix>
    LocalizingMatrix::create_matrix(const Context &context, SymbolTable& symbols,
                                    LocalizingMatrixIndex lmi,
                                    Multithreading::MultiThreadPolicy mt_policy) {

        if (context.can_have_aliases()) {
            const auto alias_lm_functor = [&context, &lmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return context.simplify_as_moment(lhs * (lmi.Word * rhs));
            };
            OperatorMatrixFactory<LocalizingMatrix, decltype(alias_lm_functor)>
                creation_context{context, symbols, lmi.Level, alias_lm_functor, mt_policy};

            return creation_context.execute(lmi);
        } else {
            const auto lm_functor = [&lmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return lhs * (lmi.Word * rhs);
            };
            OperatorMatrixFactory<LocalizingMatrix, decltype(lm_functor)>
                    creation_context{context, symbols, lmi.Level, lm_functor, mt_policy};

            return creation_context.execute(lmi);
        }

    }

}