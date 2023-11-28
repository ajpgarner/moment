/**
 * operator_matrix_creation_context.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_matrix.h"

#include "dictionary/dictionary.h"
#include "dictionary/osg_pair.h"

#include "multithreading/multithreading.h"
#include "multithreading/matrix_generation_worker.h"

#include "scenarios/context.h"

#include <cassert>

#include <memory>
#include <optional>
#include <vector>

namespace Moment {
    class MonomialMatrix;
    class SymbolicMatrix;
    class SymbolTable;

    template<std::derived_from<OperatorMatrix> os_matrix_t,
            std::derived_from<Context> context_t,
            typename index_t, typename functor_t>
    class OperatorMatrixFactory  {
    public:
        const context_t& context;

    protected:
        SymbolTable& symbols;

        functor_t elem_functor;
        const bool should_be_hermitian;

        std::optional<Multithreading::matrix_generation_worker_bundle<functor_t>> mt_bundle;

    public:
        const index_t Index;

        const std::complex<double> prefactor;

        Multithreading::MultiThreadPolicy mt_policy;

        size_t dimension = 0;
        size_t numel = 0;

        OperatorSequenceGenerator const * rowGen = nullptr;
        OperatorSequenceGenerator const * colGen = nullptr;

    public:
        std::unique_ptr<os_matrix_t> operatorMatrix;
        std::unique_ptr<MonomialMatrix> symbolicMatrix;

        explicit constexpr
        OperatorMatrixFactory(const context_t& context, SymbolTable& symbols, const index_t& matrix_index,
                              functor_t the_functor, bool should_be_hermitian, std::complex<double> prefactor,
                              const Multithreading::MultiThreadPolicy mt_policy)
            : context{context}, symbols{symbols}, elem_functor{std::move(the_functor)},
              should_be_hermitian{should_be_hermitian}, prefactor{prefactor},
              Index{matrix_index}, mt_policy{mt_policy} {
        }

        /** True, if we have determined that multithreading should be used */
        [[nodiscard]] inline constexpr bool Multithread() const noexcept {
            return mt_policy == Multithreading::MultiThreadPolicy::Always;
        }

        template<typename... Args>
        [[nodiscard]] std::unique_ptr<MonomialMatrix> execute(Args&&... args) {
            // Make or get generators
            const auto& osg_pair = functor_t::get_generators(context, this->Index); // index is already OSGIndex
            this->colGen = &(osg_pair());
            this->rowGen = &(osg_pair.conjugate());

            // Ascertain dimension
            this->dimension = colGen->size();
            assert(this->dimension == rowGen->size());
            this->numel = this->dimension * this->dimension;

            // Determine, from dimension, whether to go into MT mode.
            const bool use_multithreading
                    = Multithreading::should_multithread_matrix_creation(mt_policy, this->dimension*this->dimension);
            if (use_multithreading) {
                this->mt_policy = Multithreading::MultiThreadPolicy::Always;
                this->mt_bundle.emplace(this->context, this->symbols, *this->colGen, *this->rowGen, this->prefactor);

                this->make_operator_matrix_multi_thread(std::forward<Args>(args)...);
                this->register_new_symbols_multi_thread();
                this->make_symbolic_matrix_multi_thread();
            } else {
                this->mt_policy = Multithreading::MultiThreadPolicy::Never;
                this->make_operator_matrix_single_thread(std::forward<Args>(args)...);
                this->register_new_symbols_single_thread();
                this->make_symbolic_matrix_single_thread();
            }
            return std::move(this->symbolicMatrix);
        }

    private:
        template<typename... Args>
        void make_operator_matrix_single_thread(Args&&... params) {
            assert(colGen);
            assert(rowGen);
            std::vector<OperatorSequence> matrix_data;
            matrix_data.reserve(this->numel);
            for (const auto &colSeq: *colGen) {
                for (const auto &rowSeq: *rowGen) {
                    matrix_data.emplace_back(elem_functor(rowSeq, colSeq));
                }
            }
            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));
            this->operatorMatrix = std::make_unique<os_matrix_t>(context, std::forward<Args>(params)..., std::move(OSM));
            assert(this->operatorMatrix);
        }

        void register_new_symbols_single_thread() {

        }

        void make_symbolic_matrix_single_thread() {
            assert(this->operatorMatrix);
            if (this->prefactor != 1.0) {
                this->symbolicMatrix = std::make_unique<MonomialMatrix>(symbols,
                                                                        std::move(this->operatorMatrix), prefactor);
            } else {
                this->symbolicMatrix = std::make_unique<MonomialMatrix>(symbols, std::move(this->operatorMatrix));
            }
        }

        template<typename... Args>
        void make_operator_matrix_multi_thread(Args&&... params) {
            assert(this->colGen != nullptr);
            assert(this->rowGen != nullptr);
            assert(this->mt_bundle.has_value());

            std::vector<OperatorSequence> matrix_data{OperatorSequence::create_uninitialized_vector(this->numel)};
            this->mt_bundle->generate_operator_sequence_matrix(matrix_data.data(), this->elem_functor,
                                                               this->should_be_hermitian);

            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data),
                                                                     this->mt_bundle->non_hermitian_info());

            this->operatorMatrix = std::make_unique<os_matrix_t>(context, std::forward<Args>(params)..., std::move(OSM));
            assert(this->operatorMatrix);
        }

        void register_new_symbols_multi_thread() {
            assert(this->operatorMatrix);
            assert(this->mt_bundle.has_value());
            this->mt_bundle->identify_unique_symbols();
            this->mt_bundle->register_unique_symbols();
        }

        void make_symbolic_matrix_multi_thread() {
            assert(this->operatorMatrix);
            assert(this->mt_bundle.has_value());

            std::vector<Monomial> monomial_data(this->numel);
            this->mt_bundle->generate_symbol_matrix(monomial_data.data());
            auto mono_m_ptr = std::make_unique<SquareMatrix<Monomial>>(this->dimension, std::move(monomial_data));

            this->symbolicMatrix = std::make_unique<MonomialMatrix>(symbols,
                                                                    std::move(this->operatorMatrix),
                                                                    std::move(mono_m_ptr));
        }
    };

}