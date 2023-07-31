/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"
#include "operator_matrix_creation_context.h"

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

    class LocalizingMatrixCreationContext : public OperatorMatrixCreationContext {
    public:
        const OperatorSequence Word;

        LocalizingMatrixCreationContext(const Context& context, SymbolTable& symbols, size_t level,
                                        OperatorSequence word,
                                        Multithreading::MultiThreadPolicy mt_policy)
            : OperatorMatrixCreationContext{context, symbols, level, mt_policy}, Word{std::move(word)} {

        }

    public:
        void make_operator_matrix_single_thread() override {
            this->operatorMatrix = do_make_operator_matrix_single_thread<LocalizingMatrix>(
                    [&](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                        return lhs * (this->Word * rhs);
                    },
                    LocalizingMatrixIndex{this->Level, this->Word}
            );
            assert(this->operatorMatrix);
        }

        void make_operator_matrix_multi_thread() override {
            this->operatorMatrix = do_make_operator_matrix_multi_thread<LocalizingMatrix>(
                    [&](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                        return lhs * (this->Word * rhs);
                    },
                    LocalizingMatrixIndex{this->Level, this->Word}
            );
            assert(this->operatorMatrix);
        }
    };

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

        LocalizingMatrixCreationContext lmcc{context, symbols, lmi.Level, std::move(lmi.Word), mt_policy};
        lmcc.prepare_generators();
        lmcc.make_operator_matrix();
        lmcc.register_new_symbols();
        lmcc.make_symbolic_matrix();
        return lmcc.yield_matrix();
    }

}