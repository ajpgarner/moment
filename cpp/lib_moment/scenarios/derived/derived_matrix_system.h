/**
 * derived_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system.h"

#include <memory>

namespace Moment::Derived {
    class DerivedContext;
    class SymbolTableMap;

    class DerivedMatrixSystem : public MatrixSystem {
    public:
        /**
         * Virtual factory class for making the symbol table map.
         * The purpose of this class is to fake a virtual function call in the constructor of DerivedMatrixSystem.
         * Implementations only need to support operator() being called once.
         */
        class STMFactory {
        protected:
            STMFactory() noexcept = default;

        public:
            [[nodiscard]] virtual std::unique_ptr<SymbolTableMap>
            operator()(const SymbolTable& origin, SymbolTable& target) = 0;
        };


    private:
        /**
         * Owning pointer to base system.
         * Ownership is necessary, to prevent deletion of base system while SMS is still alive.
         */
        std::shared_ptr<MatrixSystem> base_ms_ptr;

        /** Map that defines the system */
        std::unique_ptr<Derived::SymbolTableMap> map_ptr;

    public:
        explicit DerivedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system, STMFactory&& stmf);

        virtual ~DerivedMatrixSystem() noexcept;

        /**
         * Gets the longest words in the base system that are sure to be mapped into this symbol.
         * The check can be skipped, but then this risks individual symbols triggering error::bad_map errors at a later
         * stage in the symbol matrix generations.
         */
        virtual size_t longest_supported_word() const noexcept { return std::numeric_limits<size_t>::max(); }

        /**
         * The original system this DMS is derived from.
         */
        [[nodiscard]] inline MatrixSystem& base_system() noexcept {
            return *base_ms_ptr;
        }

        /**
         * The original system this DMS is derived from.
         */
        [[nodiscard]] inline const MatrixSystem& base_system() const noexcept {
            return *base_ms_ptr;
        }

        /**
         * Map between base matrix system symbols and this system's symbols.
         */
        [[nodiscard]] inline const Derived::SymbolTableMap& map() const {
            return *this->map_ptr;
        }

        [[nodiscard]] std::string system_type_name() const override {
            return "Derived Matrix System";
        }

    protected:
        [[nodiscard]] std::unique_ptr<struct MomentMatrix>
        createNewMomentMatrix(size_t level) override;

        [[nodiscard]] std::unique_ptr<struct LocalizingMatrix>
        createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) override;

    protected:
        static std::unique_ptr<class Context> make_derived_context(const MatrixSystem& source_system);
    };
}