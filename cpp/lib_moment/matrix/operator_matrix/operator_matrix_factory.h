/**
 * operator_matrix_creation_context.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_matrix.h"
#include "operator_matrix_factory_multithreaded.h"
#include "operator_matrix_factory_singlethreaded.h"

#include "dictionary/dictionary.h"
#include "dictionary/osg_pair.h"

#include "multithreading/multithreading.h"



#include "scenarios/context.h"

#include <cassert>

#include <memory>
#include <vector>

namespace Moment {
    class MonomialMatrix;
    class SymbolicMatrix;
    class SymbolTable;

    template<typename os_matrix_t, typename context_t, typename index_t, typename functor_t>
    class OperatorMatrixFactory {
    public:
        using osg_index_t = typename index_t::OSGIndex;

        using st_factory_t = OperatorMatrixFactorySinglethreaded<os_matrix_t, context_t, index_t, functor_t>;
        using mt_factory_t = OperatorMatrixFactoryMultithreaded<os_matrix_t, context_t, index_t, functor_t>;
        using mt_factory_worker_t = OperatorMatrixFactoryWorker<os_matrix_t, context_t, index_t, functor_t>;

    public:
        /** Operator context */
        const context_t& context;

        /** Symbol table w/ write access. */
        SymbolTable& symbols;

        /** Full index, for purposes of labelling resultant matrix  */
        const index_t Index;

        /** Index, for purposes of operator sequence generator(s) */
        const osg_index_t OSGIndex;

    private:
        /** Function combining dictionary elements to matrix element (e.g. for MM, row * col). */
        functor_t elem_functor;

    public:
        /** True if the resulting matrix should, through logical arguments, be Hermitian. */
        const bool should_be_hermitian;

        /** Constant pre-factor to add in front of matrix. */
        const std::complex<double> prefactor;

        /** Which multi-threading policy should we use. */
        const Multithreading::MultiThreadPolicy mt_policy;

    private:
        /** Pointer to the conjugated dictionary "row generator". */
        OperatorSequenceGenerator const* rowGen = nullptr;

        /** Pointer to the dictionary "column generator". */
        OperatorSequenceGenerator const* colGen = nullptr;

        /** Size of matrix */
        size_t dimension = 0;

        /** True, if we cannot guarantee the resulting matrix is Hermitian (even if it should be). */
        bool could_be_non_hermitian = true;

    public:

        /**
         * Class that produces a moment matrix by first generating its operator matrix, applying any implicit symmetries,
         * identifying (and registering) the unique sequences in the matrix as symbols, and then producing the resulting
         * symbolic monomial (i.e. 'moment') matrix.
         *
         * Depending on mt_policy, this may use single or multi-threaded execution.
         *
         * @param context The operator context.
         * @param symbols The symbol table (whole matrix system should be under write lock).
         * @param matrix_index The index labelling the dictionary used to generate the matrix.
         * @param the_functor The function that combines elements from the dictionary to form matrix element operators.
         * @param should_be_hermitian True, if logically the matrix generated must be Hermitian.
         * @param prefactor Constant factor to multiply all operator sequences in matrix by.
         * @param mt_policy Should we use multi-threaded creation?
         */
        OperatorMatrixFactory(const context_t& context, SymbolTable& symbols, const index_t& matrix_index,
                              functor_t the_functor, bool should_be_hermitian, std::complex<double> prefactor,
                              const Multithreading::MultiThreadPolicy mt_policy)
                : context{context}, symbols{symbols}, elem_functor{std::move(the_functor)},
                  should_be_hermitian{should_be_hermitian}, prefactor{prefactor},
                  Index{matrix_index},
                  OSGIndex{functor_t::get_osg_index(matrix_index)},
                  mt_policy{mt_policy} {
        }

        /**
         * Do matrix creation.
         * @return Newly created matrix.
         */
        [[nodiscard]] std::unique_ptr<MonomialMatrix> execute() {
            // Make or get generators
            const auto& osg_pair = functor_t::get_generators(context, this->OSGIndex);
            this->colGen = &(osg_pair());
            this->rowGen = &(osg_pair.conjugate());
            assert(this->colGen && this->rowGen);

            // Ascertain matrix dimension & element count:
            this->dimension = colGen->size();
            assert(dimension == rowGen->size());
            const size_t numel = dimension * dimension;

            // Non-Hermitian possible, from context?
            this->could_be_non_hermitian = !should_be_hermitian
                                           || this->context.can_make_unexpected_nonhermitian_matrices();

            // Determine, from dimension and mt_policy, whether we should use multithreading:
            if (Multithreading::should_multithread_matrix_creation(mt_policy, numel)) {
                // Use multithreaded creation
                mt_factory_t factory{*this};
                return factory.execute();
            } else {
                // Use single-threaded creation
                st_factory_t factory{*this};
                return factory.execute();
            }
        }

        friend st_factory_t;
        friend mt_factory_t;
        friend mt_factory_worker_t;
    };

}