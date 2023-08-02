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
    public:
        explicit DirectSparseSTMFactory(Eigen::SparseMatrix<double> input)
            : Derived::DerivedMatrixSystem::STMFactory{}, src_matrix{std::move(input)} {

        }

        std::unique_ptr<Derived::SymbolTableMap> operator()(const SymbolTable &origin, SymbolTable &target,
                                                      Multithreading::MultiThreadPolicy mt_policy) final {
            return std::make_unique<Derived::SymbolTableMap>(origin, target,
                                                             Derived::LUMapCoreProcessor{}, this->src_matrix);
        }
    };

    class DirectDenseSTMFactory : public Derived::DerivedMatrixSystem::STMFactory {
    public:
        Eigen::MatrixXd src_matrix;
    public:
        explicit DirectDenseSTMFactory(Eigen::MatrixXd input)
            : Derived::DerivedMatrixSystem::STMFactory{}, src_matrix{std::move(input)} {

        }

        std::unique_ptr<Derived::SymbolTableMap> operator()(const SymbolTable &origin, SymbolTable &target,
                                                      Multithreading::MultiThreadPolicy mt_policy) final {
            return std::make_unique<Derived::SymbolTableMap>(origin, target,
                                                             Derived::LUMapCoreProcessor{}, this->src_matrix);
        }
    };
}
