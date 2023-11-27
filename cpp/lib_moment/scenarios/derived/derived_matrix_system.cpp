/**
 * derived_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include <sstream>

#include "derived_context.h"
#include "derived_matrix_system.h"
#include "map_core.h"
#include "symbol_table_map.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/localizing_matrix.h"

#include <cassert>

#include <sstream>
#include <tuple>

namespace Moment::errors {
    std::string make_tltte_msg(size_t max_size, size_t requested_size,
                               const std::string& object_name) {
        std::stringstream errSS;
        errSS << "Map defining derived matrix system acts on operator strings of up to length " << max_size
              << ", but words of up to length " << requested_size
              << " are required to generate " << object_name << ".";
        return errSS.str();
    }

    too_large_to_transform_error::too_large_to_transform_error(size_t max_size, size_t requested_size,
                                                               const std::string& object_name)
        : bad_transformation_error(make_tltte_msg(max_size, requested_size, object_name)) {  }
}

namespace Moment::Derived {

    namespace {
        std::unique_ptr<PolynomialMatrix> do_create_transformed_matrix(const Context& context, SymbolTable& symbols,
                                                                       double zero_tolerance,
                                                                       const SymbolTableMap& map,
                                                                       const PolynomialMatrix& source_matrix) {
            // Otherwise, resultant matrix is Polynomial
            auto symbol_mat_ptr = map(source_matrix.SymbolMatrix());

            // Create matrix
            return std::make_unique<PolynomialMatrix>(context, symbols, zero_tolerance, std::move(symbol_mat_ptr));

        }

        std::unique_ptr<SymbolicMatrix> do_create_transformed_matrix(const Context& context, SymbolTable& symbols,
                                                                     double zero_tolerance,
                                                                     const SymbolTableMap& map,
                                                                     const SymbolicMatrix& source_matrix) {
            // Monomial map on monomial matrix creates monomial matrix
            if (map.is_monomial_map() && source_matrix.is_monomial()) {
                auto mono_sym_mat_ptr = map.monomial(dynamic_cast<const MonomialMatrix &>(source_matrix).SymbolMatrix());
                return std::make_unique<MonomialMatrix>(context, symbols, zero_tolerance,
                                                        std::move(mono_sym_mat_ptr), source_matrix.Hermitian());
            }

            // Otherwise, resultant matrix is Polynomial
            auto symbol_mat_ptr = [&]() {
                if (source_matrix.is_monomial()) {
                    return map(dynamic_cast<const MonomialMatrix &>(source_matrix).SymbolMatrix());
                } else {
                    return map(dynamic_cast<const PolynomialMatrix &>(source_matrix).SymbolMatrix());
                }
            }();

            // Create matrix
            return std::make_unique<PolynomialMatrix>(context, symbols, zero_tolerance, std::move(symbol_mat_ptr));

        }
    }

    DerivedMatrixSystem::DerivedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system, STMFactory&& stm_factory,
                                             double tolerance, Multithreading::MultiThreadPolicy mt_policy)
        : MatrixSystem(DerivedMatrixSystem::make_derived_context(*base_system),
                       tolerance > 0 ? tolerance : base_system->polynomial_factory().zero_tolerance),
          derived_context{dynamic_cast<class DerivedContext&>(this->Context())},
          base_ms_ptr{std::move(base_system)}, DerivedMatrices{*this}
    {
        // Avoid deadlock. Should never occur...!
        assert(this->base_ms_ptr.get() != this);

        // Make map from factory (i.e. virtual call).
        auto lock = this->base_ms_ptr->get_read_lock();
        this->map_ptr = stm_factory(this->base_ms_ptr->Symbols(), this->Symbols(), mt_policy);
        assert(this->map_ptr != nullptr);

        // Register with context
        this->derived_context.set_symbol_table_map(this->map_ptr.get());
    }

    DerivedMatrixSystem::~DerivedMatrixSystem() noexcept = default;

    std::unique_ptr<Context> DerivedMatrixSystem::make_derived_context(const MatrixSystem& source) {
        return std::make_unique<class DerivedContext>(source.Context());
    }

    std::unique_ptr<class SymbolicMatrix>
    DerivedMatrixSystem::create_moment_matrix(MaintainsMutex::WriteLock &lock,
                                              size_t level, Multithreading::MultiThreadPolicy mt_policy) {
        // First check if map is capable of defining this MM.
        const auto lsw = this->longest_supported_word();
        if ((level*2) > lsw) {
            std::string bad_object_name{"a moment matrix of level "};
            bad_object_name.append(std::to_string(level));
            throw errors::too_large_to_transform_error{lsw, level*2, bad_object_name};
        }

        // Check source moment matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class SymbolicMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().MomentMatrix.find_index(level);
            if (index >= 0) {
                return this->base_system()[index];
            }
            read_source_lock.unlock();

            // Create new MM (will call write lock on base system).
            auto [mm_index, mm] = this->base_system().MomentMatrix.create(level, mt_policy);

            return mm;
        }();

        // NB: Cannot create new symbols.

        // Create transformation of this matrix
        return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                            this->polynomial_factory().zero_tolerance,
                                            this->map(), source_matrix);

    }

    std::unique_ptr<class SymbolicMatrix>
    DerivedMatrixSystem::create_localizing_matrix(WriteLock& lock,
                                                  const LocalizingMatrixIndex &lmi,
                                                  Multithreading::MultiThreadPolicy mt_policy) {
        // First check if map is capable of defining this LM.
        const auto lsw = this->longest_supported_word();
        const auto size_req = lmi.Level*2 + lmi.Word.size();
        if (size_req > lsw) {
            std::stringstream badNameSS;
            badNameSS << "a localizing matrix of level " << lmi.Level
                  << " for a word of length " << lmi.Word.size();
            throw errors::too_large_to_transform_error{lsw, size_req, badNameSS.str()};
        }

        // Check if source localizing matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class SymbolicMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().LocalizingMatrix.find_index(lmi);
            if (index >= 0) {
                return this->base_system()[index];
            }
            read_source_lock.unlock();

            // Create LM; will call write lock on base system
            auto [mm_index, mm] = this->base_system().LocalizingMatrix.create(lmi, mt_policy);

            return mm;
        }();

        // NB: Cannot create new symbols.

        // Create transformation of this matrix
        return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                            this->polynomial_factory().zero_tolerance,
                                            this->map(), source_matrix);
    }


    std::unique_ptr<class PolynomialMatrix>
    DerivedMatrixSystem::create_polynomial_localizing_matrix(MaintainsMutex::WriteLock& lock,
                                                             const PolynomialLMIndex& lmi,
                                                             Multithreading::MultiThreadPolicy mt_policy) {
        // Check if we can convert this matrix
        const auto lsw = this->longest_supported_word();
        const auto max_degree = this->base_system().polynomial_factory().maximum_degree(lmi.Polynomial);
        const size_t size_req = 2 * lmi.Level + max_degree;
        if (size_req > lsw) {
            std::stringstream badNameSS;
            badNameSS << "a localizing matrix of level " << lmi.Level
                      << " for a polynomial of degree " << max_degree;
            throw errors::too_large_to_transform_error{lsw, size_req, badNameSS.str()};
        }

        // Ensure source localizing matrix exists
        const auto& source_matrix = [&]() -> const class PolynomialMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto offset = this->base_system().PolynomialLocalizingMatrix.find_index(lmi);
            if (offset >= 0) {
                return dynamic_cast<const PolynomialMatrix&>(this->base_system()[offset]); // ~read_source_lock
            }
            read_source_lock.unlock();

            // Create LM; will call write lock on base system
            auto [mm_index, mm] = this->base_system().PolynomialLocalizingMatrix.create(lmi, mt_policy);

            return mm;
        }();

        // Create transformation of this matrix
        return do_create_transformed_matrix(this->Context(), this->Symbols(), this->polynomial_factory().zero_tolerance,
                                            this->map(), source_matrix);
    }

    std::unique_ptr<class SymbolicMatrix>
    DerivedMatrixSystem::create_derived_matrix(MaintainsMutex::WriteLock& lock, ptrdiff_t source_offset,
                                               Multithreading::MultiThreadPolicy mt_policy) {
        // Read from source
        auto read_source_lock = this->base_system().get_read_lock();
        if ((source_offset < 0) || (source_offset >= this->base_system().size())) {
            throw errors::bad_transformation_error{"Cannot transform matrix that does not exist in base system."};
            // ~read_source_lock
        }
        const auto& src_matrix = this->base_system()[source_offset];

        // Do transformation
        if (src_matrix.is_monomial()) {
            const auto& mono_matrix = dynamic_cast<const MonomialMatrix&>(src_matrix);
            return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                                this->polynomial_factory().zero_tolerance,
                                                this->map(), mono_matrix);
        } else {
            const auto& poly_matrix = dynamic_cast<const PolynomialMatrix&>(src_matrix);
            return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                                this->polynomial_factory().zero_tolerance,
                                                this->map(), poly_matrix);
        }
    }


    void DerivedMatrixSystem::on_new_moment_matrix(const MaintainsMutex::WriteLock& write_lock, size_t level,
                                                   ptrdiff_t sym_offset, const SymbolicMatrix& mm) {

        auto base_offset = this->base_system().MomentMatrix.find_index(level);
        assert(base_offset >= 0);
        auto actual_offset = this->DerivedMatrices.insert_alias(write_lock, base_offset, sym_offset);
        assert(actual_offset == sym_offset);

    }

    void DerivedMatrixSystem::on_new_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                       const LocalizingMatrixIndex& lmi, ptrdiff_t sym_offset,
                                                       const SymbolicMatrix& lm) {
        auto base_offset = this->base_system().LocalizingMatrix.find_index(lmi);
        assert(base_offset >= 0);
        auto actual_offset = this->DerivedMatrices.insert_alias(write_lock, base_offset, sym_offset);
        assert(actual_offset == sym_offset);
    }

    void DerivedMatrixSystem::on_new_polynomial_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                  const PolynomialLMIndex& lmi, ptrdiff_t sym_offset,
                                                                  const PolynomialMatrix& plm) {
        auto base_offset = this->base_system().PolynomialLocalizingMatrix.find_index(lmi);
        assert(base_offset >= 0);
        auto actual_offset = this->DerivedMatrices.insert_alias(write_lock, base_offset, sym_offset);
        assert(actual_offset == sym_offset);
    }

    void DerivedMatrixSystem::on_new_derived_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                    ptrdiff_t source_offset, ptrdiff_t target_offset,
                                                    const SymbolicMatrix& target_matrix) {
        // TODO: Reflection to determine if source matrix is a moment matrix, localizing matrix, or P-LM, etc.

    }

    std::string DerivedMatrixSystem::describe_map() const {
        std::stringstream msgSS;
        size_t bs_size = this->base_system().Symbols().size();
        size_t bs_actual = this->map().fwd_size();
        msgSS << "Map from " << this->base_system().system_type_name()
              << " with " << bs_size << " " << ((bs_size != 1) ? "symbols" : "symbol");
        if (bs_actual != bs_size) {
            msgSS << " [" << bs_actual << " defined" << "]";
        }
        msgSS << " to ";

        size_t this_size = this->Symbols().size();
        size_t this_actual = this->map().inv_size();
        msgSS << this->system_type_name() << " with "
              <<  this_size << " " << ((this_size != 1) ? "symbols" : "symbol");
        if (this_size != this_actual) {
            msgSS << " [" << this_actual << " defined" << "]";
        }
        msgSS << ".";

        return msgSS.str();
    }

}