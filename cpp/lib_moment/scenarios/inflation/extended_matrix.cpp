/**
 * extended_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "extended_matrix.h"
#include "scenarios/inflation/multithreading/extended_matrix_worker.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"
#include "dictionary/operator_sequence_generator.h"

#include "factor_table.h"
#include "inflation_context.h"

#include <algorithm>
#include <sstream>


namespace Moment::Inflation {

    namespace {

        std::string make_description(const size_t level, const std::span<const symbol_name_t> extensions) {
            std::stringstream ss;
            ss << "Extended Moment Matrix, Level " <<  level << ", Extensions ";
            bool done_one = false;
            for (auto ext : extensions) {
                if (done_one) {
                    ss << ", ";
                } else {
                    done_one = true;
                }
                ss << "S" << ext;
            }
            return ss.str();
        }

        symbol_name_t combine_and_register_factors(SymbolTable &symbols, FactorTable &factors,
                                                   const std::vector<symbol_name_t>& source_factors,
                                                   const std::vector<symbol_name_t>& extended_factors) {
            auto joint_factors = FactorTable::combine_symbolic_factors(source_factors, extended_factors);
            auto maybe_symbol_index = factors.find_index_by_factors(joint_factors);

            // Push in new factors to symbol table, if not already there
            if (!maybe_symbol_index.has_value()) {
                maybe_symbol_index = symbols.create(true, false);
                factors.register_new(*maybe_symbol_index, joint_factors);
            }

            return maybe_symbol_index.value();
        }


        std::unique_ptr<SquareMatrix<Monomial>>
        make_extended_matrix_single_thread(const InflationContext& context,
                                           SymbolTable& symbols,
                                           Inflation::FactorTable& factors,
                                           const MonomialMatrix& source,
                                           const MomentMatrix& moment_matrix,
                                           const std::span<const symbol_name_t> extension_scalars) {

            // Check scalars are in range
            for (const auto scalar : extension_scalars) {
                if ((scalar >= symbols.size()) || (scalar < 0)) {
                    std::stringstream errSS;
                    errSS << "Cannot extend matrix with unknown symbol \"" << scalar << "\".";
                    throw std::logic_error{errSS.str()};
                }
            }

            // Create matrix, with source matrix as upper block
            const size_t padding = extension_scalars.size();
            auto extended_matrix = source.SymbolMatrix().pad(padding, Monomial{0});

            const size_t old_dimension = source.Dimension();
            const size_t new_dimension = source.Dimension() + padding;
            assert(new_dimension == extended_matrix.dimension);

            // Existing generators, combine with scalars...
            const auto& mm_osg = moment_matrix.generators()();

            size_t row_index = 0;
            for (const auto& raw_seq : mm_osg) {
                // Get canonical version of sequence...
                auto seq = context.canonical_moment(raw_seq);

                auto [source_sym_index, source_conj] = symbols.hash_to_index(seq.hash());
                assert(!source_conj); // No symbols should be conjugated in entirely commutative, Hermitian setting...!
                assert(source_sym_index != std::numeric_limits<ptrdiff_t>::max()); // Must find symbol in table.

                // Copy, because of possible re-allocation within factor table!
                const auto source_factors = factors[source_sym_index].canonical.symbols;
                size_t col_index = old_dimension;
                for (auto scalar_symbol_id : extension_scalars) {
                    // Calculate factors
                    const auto& extended_factors = factors[scalar_symbol_id].canonical.symbols;
                    symbol_name_t factor_id = combine_and_register_factors(symbols, factors,
                                                                           source_factors, extended_factors);

                    // Write in symbol matrix
                    extended_matrix(std::array<size_t, 2>{row_index, col_index}) = Monomial{factor_id};
                    extended_matrix(std::array<size_t, 2>{col_index, row_index}) = Monomial{factor_id};

                    ++col_index;
                }
                assert(col_index == new_dimension);
                ++row_index;
            }
            assert(row_index == old_dimension);

            // Make bottom-right block
            for (size_t i = 0, iMax = extension_scalars.size(); i < iMax; ++i) {
                const auto& row_factors = factors[extension_scalars[i]].canonical.symbols;
                auto diag_fac_id = combine_and_register_factors(symbols, factors, row_factors, row_factors);
                const std::array<size_t,2> diag_index{old_dimension+i, old_dimension+i};
                extended_matrix(diag_index) = Monomial{diag_fac_id};

                for (size_t j = i+1; j < iMax; ++j) {
                    const auto& col_factors = factors[extension_scalars[j]].canonical.symbols;
                    auto offdiag_fac_id = combine_and_register_factors(symbols, factors, row_factors, col_factors);
                    const std::array<size_t,2> upper_index{old_dimension+i, old_dimension+j};
                    extended_matrix(upper_index) = Monomial{offdiag_fac_id};
                    const std::array<size_t,2> lower_index{old_dimension+j, old_dimension+i};
                    extended_matrix(lower_index) = Monomial{offdiag_fac_id};
                }
            }
            return std::make_unique<SquareMatrix<Monomial>>(std::move(extended_matrix));
        }


        std::unique_ptr<SquareMatrix<Monomial>>
        inline make_extended_matrix_multi_thread(const InflationContext& context,
                                           SymbolTable& symbols,
                                           Inflation::FactorTable& factors,
                                           const MonomialMatrix& source,
                                           const MomentMatrix& moment_matrix,
                                           const std::span<const symbol_name_t> extension_scalars) {
            Multithreading::ExtendedMatrixBundle factory{context, symbols, factors,
                                                         source, moment_matrix, extension_scalars};
            return factory.execute();
        }

        std::unique_ptr<SquareMatrix<Monomial>>
        make_extended_matrix(SymbolTable& symbols, Inflation::FactorTable& factors,
                             const MonomialMatrix& source,
                             const std::span<const symbol_name_t> extension_scalars,
                             const Multithreading::MultiThreadPolicy mt_policy) {

            // Moment matrix must come from inflation context
            const auto& context = dynamic_cast<const InflationContext&>(source.context); // throws if invalid!

            // Source matrix must be moment matrix
            MomentMatrix const * mm_ptr = MomentMatrix::to_operator_matrix_ptr(source);
            if (nullptr == mm_ptr) {
                throw std::invalid_argument{"Can only extend monomial moment matrices."};
            }
            const auto& moment_matrix = *mm_ptr;

            // Check source is Hermitian
            if (!source.Hermitian()) {
                throw std::invalid_argument{"Scalar extension of non-Hermitian matrices is not supported."};
            }

            // Get target dimension
            const size_t target_dimension = source.Dimension() + extension_scalars.size();
            const bool should_multithread
                = Multithreading::should_multithread_matrix_creation(mt_policy, target_dimension * target_dimension);

            if (should_multithread) {
                return make_extended_matrix_multi_thread(context, symbols, factors,
                                                          source, moment_matrix, extension_scalars);
            } else {
                return make_extended_matrix_single_thread(context, symbols, factors,
                                                          source, moment_matrix, extension_scalars);
            };
        }
    }

    ExtendedMatrix::ExtendedMatrix(SymbolTable& symbols, Inflation::FactorTable& factors, const double zero_tolerance,
                                   const MonomialMatrix &source,
                                   const std::span<const symbol_name_t> extensions,
                                   const Multithreading::MultiThreadPolicy mt_policy)
        : MonomialMatrix{source.context, symbols, zero_tolerance,
                         make_extended_matrix(symbols, factors, source, extensions, mt_policy),
                         source.Hermitian()},
          OriginalDimension{source.Dimension()} {

        MomentMatrix const * mm_ptr = MomentMatrix::to_operator_matrix_ptr(source);
        assert(mm_ptr); // ^- make_extended_matrix should have already thrown exception if above is nullptr!

        // Make description string of extended matrix
        this->description = make_description(mm_ptr->Index, extensions);
    }


}