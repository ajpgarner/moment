/**
 * symmetrized_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symmetrized_matrix_system.h"
#include "group.h"

#include "../derived/derived_context.h"
#include "../derived/map_core.h"
#include "../derived/symbol_table_map.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/localizing_matrix.h"

#include <cassert>
#include <sstream>

namespace Moment::Symmetrized {
    SymmetrizedMatrixSystem::SymmetrizedSTMFactory::SymmetrizedSTMFactory(
            Group& group, size_t max_word_length, std::unique_ptr<Derived::MapCoreProcessor>&& proc_in) noexcept
          : group{group}, max_word_length{max_word_length}, processor{std::move(proc_in)}  {
        assert(static_cast<bool>(this->processor));
    }

    std::unique_ptr<Derived::SymbolTableMap>
    SymmetrizedMatrixSystem::SymmetrizedSTMFactory::make(const SymbolTable& origin_symbols,
                                                         SymbolTable& target_symbols,
                                                         Multithreading::MultiThreadPolicy mt_policy) {
        // First, ensure source defines enough symbols to do generation
        const auto osg_length = origin_symbols.OSGIndex.max_length();
        if (osg_length < max_word_length) {
            std::stringstream errSS;
            errSS << "Could not generate map for strings of length " << max_word_length
                 << ", because origin symbol table has only been populated up to strings of length " << osg_length;
            throw errors::bad_map{errSS.str()};
        }

        // Next, ensure group has representation for requested length.
        const auto& group_rep = this->group.create_representation(max_word_length, mt_policy);

        // Next, get average of group action in this representation
        const repmat_t average = group_rep.sum_of() / static_cast<double>(group.size);

        // Do processing
        return std::make_unique<Derived::SymbolTableMap>(origin_symbols, target_symbols, *this->processor, average);
    }

    SymmetrizedMatrixSystem::SymmetrizedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system,
                                                     std::unique_ptr<Group>&& group,
                                                     const size_t max_word_length,
                                                     std::unique_ptr<Derived::MapCoreProcessor>&& processor,
                                                     double tolerance,
                                                     Multithreading::MultiThreadPolicy mt_policy)
        : Derived::DerivedMatrixSystem{std::move(base_system),
                                       SymmetrizedSTMFactory{*group, max_word_length, std::move(processor)},
                                       tolerance, mt_policy},
          symmetry{std::move(group)}, max_word_length{max_word_length} {

    }

    SymmetrizedMatrixSystem::~SymmetrizedMatrixSystem() noexcept = default;

}