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

#include "derived_context.h"
#include "symbol_table_map.h"

#include "matrix/moment_matrix.h"
#include "matrix/localizing_matrix.h"

#include <cassert>

namespace Moment::Derived {
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

    std::unique_ptr<class MomentMatrix> DerivedMatrixSystem::createNewMomentMatrix(size_t level) {
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
        const auto& source_matrix = [&]() -> const class MomentMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_moment_matrix(level);
            if (index >= 0) {
                return dynamic_cast<const class MomentMatrix&>(this->base_system()[index]);
            }
            read_source_lock.unlock();

            // Wait for write lock...
            auto write_source_lock = this->base_system().get_write_lock();
            auto [mm_index, mm] = this->base_system().create_moment_matrix(level);

            return mm; // write_source_lock unlocks
        }();


        throw std::runtime_error{"DerivedMatrixSystem::createNewMomentMatrix not implemented."};
    }

    std::unique_ptr<class LocalizingMatrix>
    DerivedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) {
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
        const auto& source_matrix = [this, &lmi]() -> const class LocalizingMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_localizing_matrix(lmi);
            if (index >= 0) {
                return dynamic_cast<const class LocalizingMatrix&>(this->base_system()[index]);
            }
            read_source_lock.unlock();

            // Wait for write lock...
            auto write_source_lock = this->base_system().get_write_lock();
            auto [mm_index, mm] = this->base_system().create_localizing_matrix(lmi);

            return mm; // write_source_lock unlocks
        }();

        throw std::runtime_error{"DerivedMatrixSystem::createNewLocalizingMatrix not implemented."};
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