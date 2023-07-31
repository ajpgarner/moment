/**
 * matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "matrix_system.h"

#include "polynomial_index_storage.h"

#include "dictionary/dictionary.h"

#include "matrix/operator_matrix/localizing_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/polynomial_localizing_matrix.h"
#include "matrix/substituted_matrix.h"

#include "scenarios/context.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Moment {
    namespace {
        const Context& assertContext(const std::unique_ptr<Context>& contextIn) {
            assert(contextIn);
            return *contextIn;
        }
    }

    MatrixSystem::MatrixSystem(std::unique_ptr<class Context> ctxtIn, const double zero_tolerance)
        : context{std::move(ctxtIn)}, symbol_table{std::make_unique<SymbolTable>(assertContext(context))},
          MomentMatrix{*this},
          LocalizingMatrix{*this},
          PolynomialLocalizingMatrix{*this},
          SubstitutedMatrix{*this},
          Rulebook{*this} {
        this->poly_factory = std::make_unique<ByIDPolynomialFactory>(*this->symbol_table, zero_tolerance);
        this->PolynomialLocalizingMatrix.indices.set_factory(*this->poly_factory);
    }

    MatrixSystem::~MatrixSystem() noexcept = default;

    const SymbolicMatrix &MatrixSystem::operator[](size_t index) const {
        if (index >= this->matrices.size()) {
            std::stringstream errSS;
            errSS << "Matrix index " << index << " is out of range (max index: " << (this->matrices.size()-1) << ").";
            throw errors::missing_component{errSS.str()};
        }
        if (!this->matrices[index]) [[unlikely]] {
            std::stringstream errSS;
            errSS << "Matrix at index " << index << " was missing.";
            throw errors::missing_component(errSS.str());
        }
        return *this->matrices[index];
    }

    SymbolicMatrix& MatrixSystem::get(size_t index) {
        if (index >= this->matrices.size()) {
            std::stringstream errSS;
            errSS << "Matrix index " << index << " is out of range (max index: " << (this->matrices.size()-1) << ").";
            throw errors::missing_component{errSS.str()};
        }
        if (!this->matrices[index]) [[unlikely]] {
            std::stringstream errSS;
            errSS << "Matrix at index " << index << " was missing.";
            throw errors::missing_component(errSS.str());
        }

        return *this->matrices[index];
    }

    ptrdiff_t MatrixSystem::push_back(std::unique_ptr<SymbolicMatrix> matrix) {
        auto matrixIndex = static_cast<ptrdiff_t>(this->matrices.size());
        this->matrices.emplace_back(std::move(matrix));
        return matrixIndex;
    }

    std::unique_ptr<class SymbolicMatrix>
    MatrixSystem::createNewMomentMatrix(MaintainsMutex::WriteLock &lock,
                                        const size_t level, const Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));
        return MomentMatrix::create_matrix(*this->context, *this->symbol_table, level, mt_policy);
    }


    std::unique_ptr<class SymbolicMatrix>
    MatrixSystem::createNewLocalizingMatrix(MaintainsMutex::WriteLock &lock,
                                            const LocalizingMatrixIndex& lmi,
                                            const Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));
        return LocalizingMatrix::create_matrix(*this->context, *this->symbol_table, lmi, mt_policy);
    }

    std::unique_ptr<class PolynomialMatrix>
    MatrixSystem::createNewPolyLM(MaintainsMutex::WriteLock &lock,
                                  const PolynomialLMIndex &index, Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));

        // First ensure constituent parts exist
        PolynomialLocalizingMatrix::Constituents constituents;
        constituents.reserve(index.Polynomial.size());
        for (auto [mono_index, factor] : index.MonomialIndices(*this->symbol_table)) {
            auto [mono_offset, mono_matrix] = this->LocalizingMatrix.create(lock, mono_index, mt_policy);
            constituents.emplace_back(&mono_matrix, factor);
        }

        // Synthesize into polynomial matrix
        return std::make_unique<class PolynomialLocalizingMatrix>(*this->context, *this->symbol_table,
                                                                  *this->poly_factory,
                                                                  PolynomialLMIndex{index}, std::move(constituents));
    }

    bool MatrixSystem::generate_dictionary(const size_t word_length) {
        auto write_lock = this->get_write_lock();

        auto [osg_size, new_symbols] = this->symbol_table->fill_to_word_length(word_length);

        this->onDictionaryGenerated(word_length, this->context->operator_sequence_generator(word_length));

        return new_symbols;
    }


    void MatrixSystem::replace_polynomial_factory(std::unique_ptr<PolynomialFactory> new_factory) {
        assert(new_factory);
        if (!this->matrices.empty() || !this->Rulebook.empty()) [[unlikely]] {
            throw std::runtime_error{"Cannot change polynomial factory after matrix system is in use."};
        }
        this->poly_factory = std::move(new_factory);
        this->PolynomialLocalizingMatrix.indices.set_factory(*this->poly_factory);
    }

}