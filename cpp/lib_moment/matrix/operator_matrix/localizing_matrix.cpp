/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"
#include "dictionary/dictionary.h"

#include <sstream>

namespace Moment {


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

//    std::unique_ptr<MonomialMatrix>
//    LocalizingMatrix::create_matrix(const Context &context, SymbolTable& symbols,
//                                    LocalizingMatrixIndex lmi,
//                                    Multithreading::MultiThreadPolicy mt_policy) {
//
//        const bool should_be_hermitian = (lmi.Word.hash() == lmi.Word.conjugate().hash())
//                                            && !is_imaginary(lmi.Word.get_sign());
//
//        if (context.can_have_aliases()) {
//            const auto alias_lm_functor = [&context, &lmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
//                return context.simplify_as_moment(lhs * (lmi.Word * rhs));
//            };
//            OperatorMatrixFactory<LocalizingMatrix, class Context, size_t, decltype(alias_lm_functor)>
//                creation_context{context, symbols, lmi.Level, alias_lm_functor, should_be_hermitian, 1.0, mt_policy};
//
//            return creation_context.execute(lmi);
//        } else {
//            const auto lm_functor = [&lmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
//                return lhs * (lmi.Word * rhs);
//            };
//            OperatorMatrixFactory<LocalizingMatrix, class Context, size_t, decltype(lm_functor)>
//                    creation_context{context, symbols, lmi.Level, lm_functor, should_be_hermitian, 1.0, mt_policy};
//
//            return creation_context.execute(lmi);
//        }
//
//    }

}