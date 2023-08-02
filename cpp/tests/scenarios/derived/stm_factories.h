/**
 * stm_factories.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "scenarios/derived/derived_matrix_system.h"
#include "scenarios/derived/symbol_table_map.h"
#include "scenarios/derived/lu_map_core_processor.h"

#include <Eigen/SparseCore>

namespace Moment::Tests {

    class DirectSparseSTMFactory : public Derived::DerivedMatrixSystem::STMFactory {
    public:
        Eigen::SparseMatrix<double> src_matrix;
        const size_t max_wl;

    public:
        explicit DirectSparseSTMFactory(Eigen::SparseMatrix<double> input, size_t max_wl)
            : Derived::DerivedMatrixSystem::STMFactory{}, src_matrix{std::move(input)}, max_wl{max_wl} {

        }

        std::unique_ptr<Derived::SymbolTableMap> operator()(SymbolTable &origin, SymbolTable &target,
                                                      Multithreading::MultiThreadPolicy mt_policy) final {
            origin.OSGIndex.update_if_necessary(this->max_wl);
            return std::make_unique<Derived::SymbolTableMap>(origin, target,
                                                             Derived::LUMapCoreProcessor{}, this->src_matrix);
        }
    };

    class DirectDenseSTMFactory : public Derived::DerivedMatrixSystem::STMFactory {
    public:
        size_t max_wl;
        Eigen::MatrixXd src_matrix;
    public:
        explicit DirectDenseSTMFactory(Eigen::MatrixXd input, size_t max_wl)
            : Derived::DerivedMatrixSystem::STMFactory{}, src_matrix{std::move(input)}, max_wl{max_wl} {

        }

        std::unique_ptr<Derived::SymbolTableMap> operator()(SymbolTable &origin, SymbolTable &target,
                                                      Multithreading::MultiThreadPolicy mt_policy) final {
            origin.OSGIndex.update_if_necessary(this->max_wl);
            return std::make_unique<Derived::SymbolTableMap>(origin, target,
                                                             Derived::LUMapCoreProcessor{}, this->src_matrix);
        }
    };
}
