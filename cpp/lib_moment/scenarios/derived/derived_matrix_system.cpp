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

#include "symbolic/polynomial_factory.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/localizing_matrix.h"

#include <cassert>

namespace Moment::Derived {

    namespace {
        std::unique_ptr<Matrix> do_create_transformed_matrix(const Context& context, SymbolTable& symbols,
                                                             double zero_tolerance,
                                                             const SymbolTableMap& map, const Matrix& source_matrix) {
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

    DerivedMatrixSystem::DerivedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system, STMFactory&& stm_factory)
        : MatrixSystem(DerivedMatrixSystem::make_derived_context(*base_system)),
          base_ms_ptr{std::move(base_system)}
    {
        // Avoid deadlock. Should never occur...!
        assert(this->base_ms_ptr.get() != this);

        // Make map from factory (i.e. virtual call).
        auto lock = this->base_ms_ptr->get_read_lock();
        this->map_ptr = stm_factory(this->base_ms_ptr->Symbols(), this->Symbols());

    }

    DerivedMatrixSystem::~DerivedMatrixSystem() noexcept = default;

    std::unique_ptr<Context> DerivedMatrixSystem::make_derived_context(const MatrixSystem& source) {
        return std::make_unique<Derived::DerivedContext>(source.Context());
    }

    std::unique_ptr<class Matrix>
    DerivedMatrixSystem::createNewMomentMatrix(size_t level, Multithreading::MultiThreadPolicy mt_policy) {
        // First check if map is capable of defining this MM.
        const auto lsw = this->longest_supported_word();
        if ((level*2) > lsw) {
            std::stringstream errSS;
            errSS << "Map defining derived matrix system acts on operator strings of up to length " << lsw
                << ", but words of up to length " << (level*2)
                << " are required to generate a moment matrix of level " << level << ".";
            throw errors::bad_map{errSS.str()};
        }

        // Check source moment matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class Matrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_moment_matrix(level);
            if (index >= 0) {
                return this->base_system()[index];
            }
            read_source_lock.unlock();

            // Create new MM (will call write lock on base system).
            auto [mm_index, mm] = this->base_system().create_moment_matrix(level, mt_policy);

            return mm;
        }();

        // Create transformation of this matrix
        return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                            this->polynomial_factory().zero_tolerance,
                                            this->map(), source_matrix);
    }

    std::unique_ptr<class Matrix>
    DerivedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi,
                                                   Multithreading::MultiThreadPolicy mt_policy) {
        // First check if map is capable of defining this LM.
        const auto lsw = this->longest_supported_word();
        const auto size_req = lmi.Level*2 + lmi.Word.size();
        if (size_req > lsw) {
            std::stringstream errSS;
            errSS << "Map defining derived matrix system acts on operator strings of up to length " << lsw
                  << ", but words of up to length " << size_req
                  << " are required to generate a localizing matrix of level " << lmi.Level
                  << " for a word of length " << lmi.Word.size() << ".";
            throw errors::bad_map{errSS.str()};
        }

        // Check if source localizing matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class Matrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_localizing_matrix(lmi);
            if (index >= 0) {
                return this->base_system()[index];
            }
            read_source_lock.unlock();

            // Create LM; will call write lock on base system
            auto [mm_index, mm] = this->base_system().create_localizing_matrix(lmi, mt_policy);

            return mm;
        }();

        // Create transformation of this matrix
        return do_create_transformed_matrix(this->Context(), this->Symbols(),
                                            this->polynomial_factory().zero_tolerance,
                                            this->map(), source_matrix);
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