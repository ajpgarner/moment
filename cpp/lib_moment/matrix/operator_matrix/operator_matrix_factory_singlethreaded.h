/**
 * singlethreaded_operator_matrix_factory.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "is_hermitian.h"
#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"
#include "matrix/monomial_matrix.h"
#include "scenarios/context.h"

#include <memory>

namespace Moment {
    template<typename os_matrix_t, typename context_t, typename index_t, typename functor_t>
    class OperatorMatrixFactory;

    template<typename os_matrix_t, typename context_t, typename index_t, typename elem_functor_t>
    class OperatorMatrixFactorySinglethreaded {
        public:
            using info_factory_t = OperatorMatrixFactory<os_matrix_t, context_t, index_t, elem_functor_t>;

            info_factory_t& factory;

        public:
            explicit OperatorMatrixFactorySinglethreaded(info_factory_t& factory)
                : factory{factory} {

            }


            /**
             * Do multi-stage matrix generation.
             * NB: Only one thread should call execute at once!
             */
            std::unique_ptr<MonomialMatrix> execute() {
                std::unique_ptr<os_matrix_t> unaliased_matrix = this->make_operator_matrix();

                std::unique_ptr<os_matrix_t> aliased_matrix;
                if (this->factory.context.can_have_aliases()) {
                    aliased_matrix = this->make_aliased_operator_matrix(*unaliased_matrix);
                }

                return this->make_symbolic_matrix_single_thread(std::move(unaliased_matrix), std::move(aliased_matrix));
            }


    private:
        std::unique_ptr<os_matrix_t> make_operator_matrix() {
            // Generate unaliased matrix
            std::vector<OperatorSequence> matrix_data;
            matrix_data.reserve(this->factory.dimension * this->factory.dimension);
            for (const auto &colSeq: *factory.colGen) {
                for (const auto &rowSeq: *factory.rowGen) {
                    matrix_data.emplace_back(factory.elem_functor(rowSeq, colSeq));
                }
            }
            auto OSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(factory.dimension, std::move(matrix_data));
            return std::make_unique<os_matrix_t>(factory.context, factory.Index, std::move(OSM));

        }

        std::unique_ptr<os_matrix_t>
        make_aliased_operator_matrix(const os_matrix_t& unaliased_matrix) {
            // Generate aliased operator matrix, if this could exist
            std::vector<OperatorSequence> aliased_data;
            aliased_data.reserve(this->factory.dimension * this->factory.dimension);
            const auto& context = this->factory.context;

            std::transform( unaliased_matrix().cbegin(), unaliased_matrix().cend(),
                            std::back_inserter(aliased_data),
                            [&context](const OperatorSequence& unaliased_sequence) {
                        return context.simplify_as_moment(unaliased_sequence);
                    });
            auto aOSM = std::make_unique<OperatorMatrix::OpSeqMatrix>(factory.dimension, std::move(aliased_data));
            return std::make_unique<os_matrix_t>(factory.context, factory.Index, std::move(aOSM));
        }

        std::unique_ptr<MonomialMatrix>
        make_symbolic_matrix_single_thread(std::unique_ptr<os_matrix_t> unaliased_matrix,
                                           std::unique_ptr<os_matrix_t> aliased_matrix) {
            assert(unaliased_matrix);
            if (this->factory.context.can_have_aliases()) {
                assert(aliased_matrix);
                if (this->factory.prefactor != 1.0) {
                    return std::make_unique<MonomialMatrix>(factory.symbols,
                                                            std::move(unaliased_matrix), std::move(aliased_matrix),
                                                            factory.prefactor);
                } else {
                    return std::make_unique<MonomialMatrix>(factory.symbols,
                                                            std::move(unaliased_matrix), std::move(aliased_matrix));
                }
            } else {
                assert(aliased_matrix == nullptr);
                if (this->factory.prefactor != 1.0) {
                    return std::make_unique<MonomialMatrix>(factory.symbols, std::move(unaliased_matrix),
                                                            factory.prefactor);
                } else {
                    return std::make_unique<MonomialMatrix>(factory.symbols, std::move(unaliased_matrix));
                }
            }
        }
    };

}