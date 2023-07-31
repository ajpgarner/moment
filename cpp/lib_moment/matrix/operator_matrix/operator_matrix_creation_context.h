/**
 * operator_matrix_creation_context.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/multithreading.h"
#include "multithreading/matrix_generation_worker.h"

#include "scenarios/context.h"

#include <cassert>

#include <memory>
#include <vector>

namespace Moment {
    class MonomialMatrix;
    class SymbolicMatrix;
    class SymbolTable;

    class OperatorMatrixCreationContext  {
    public:
        const Context& context;
    protected:
        SymbolTable& symbols;

    public:
        const size_t Level;

        Multithreading::MultiThreadPolicy mt_policy;
        size_t dimension = 0;
        const OperatorSequenceGenerator* rowGen = nullptr;
        const OperatorSequenceGenerator* colGen = nullptr;

    public:
        std::unique_ptr<OperatorMatrix> operatorMatrix;
        std::unique_ptr<MonomialMatrix> symbolicMatrix;

        explicit OperatorMatrixCreationContext(const Context& context, SymbolTable& symbols, size_t level,
                                               const Multithreading::MultiThreadPolicy mt_policy);

        virtual ~OperatorMatrixCreationContext() noexcept;

        /** True, if we have determined that multithreading should be used */
        [[nodiscard]] inline constexpr bool Multithread() const noexcept {
            return mt_policy == Multithreading::MultiThreadPolicy::Always;
        }

        void prepare_generators();

        void make_operator_matrix();

        void register_new_symbols();

        void make_symbolic_matrix();

        std::unique_ptr<MonomialMatrix> yield_matrix() noexcept;

    protected:
        virtual void make_operator_matrix_single_thread() = 0;

        virtual void make_operator_matrix_multi_thread() = 0;

    private:
        void register_new_symbols_single_thread();

        void register_new_symbols_multi_thread();

        void make_symbolic_matrix_single_thread();

        void make_symbolic_matrix_multi_thread();

    protected:
        template<typename output_t, typename functor_t, typename... Args>
        std::unique_ptr<output_t> do_make_operator_matrix_single_thread(functor_t functor, Args&&... params) {
            assert(colGen);
            assert(rowGen);
            std::vector<OperatorSequence> matrix_data;
            matrix_data.reserve(dimension * dimension);
            if (context.can_have_aliases()) {
                for (const auto &colSeq: *colGen) {
                    for (const auto &rowSeq: *rowGen) {
                        matrix_data.emplace_back(context.simplify_as_moment(functor(rowSeq, colSeq)));
                    }
                }
            } else {
                for (const auto &colSeq: *colGen) {
                    for (const auto &rowSeq: *rowGen) {
                        matrix_data.emplace_back(functor(rowSeq, colSeq));
                    }
                }
            }
            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));
            return std::make_unique<output_t>(context, std::forward<Args>(params)..., std::move(OSM));
        }

        template<typename output_t, typename functor_t, typename... Args>
        std::unique_ptr<output_t> do_make_operator_matrix_multi_thread(functor_t functor, Args&&... params) {
            assert(colGen != nullptr);
            assert(rowGen != nullptr);
            const size_t numel = this->dimension * this->dimension;

            std::vector<OperatorSequence> matrix_data{OperatorSequence::create_uninitialized_vector(numel)};
            if (context.can_have_aliases()) {
                auto wrapped_functor = [&](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                    return this->context.simplify_as_moment(functor(lhs, rhs));
                };

                Multithreading::generate_matrix_data(*colGen, *rowGen, matrix_data.data(), wrapped_functor);
            } else {
                Multithreading::generate_matrix_data(*colGen, *rowGen, matrix_data.data(), functor);
            }

            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));
            return std::make_unique<output_t>(context, std::forward<Args>(params)..., std::move(OSM));
        }
    };

}