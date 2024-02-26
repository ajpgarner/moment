/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "storage_manager.h"

#include "import/read_localizing_matrix_indices.h"
#include "import/read_opseq_polynomial.h"

#include "matrix/operator_matrix/localizing_matrix.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/derived/derived_matrix_system.h"

#include "utilities/reporting.h"
#include "utilities/io_parameters.h"
#include "matrix/polynomial_matrix.h"

#include <memory>


namespace Moment::mex::functions {

    LocalizingMatrixParams::LocalizingMatrixParams(SortedInputs &&inputs)
    : OperatorMatrixParams(std::move(inputs)) {
        this->lmi_importer_ptr = std::make_unique<LocalizingMatrixIndexImporter>(this->matlabEngine);
    }

    LocalizingMatrixParams::~LocalizingMatrixParams() noexcept = default;


    void LocalizingMatrixParams::extra_parse_params() {
        assert(inputs.empty()); // Should be guaranteed by parent.

        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // Get depth
        auto& depth_param = this->find_or_throw(u"level");
        this->lmi_importer_ptr->read_level(depth_param);


        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get localizing word sequence
        auto& word_param = this->find_or_throw(u"word");
        this->lmi_importer_ptr->read_localizing_expression(word_param, expr_type);

        this->parse_optional_params();
    }

    void LocalizingMatrixParams::extra_parse_inputs() {
        // Do we offset by -1?
        if (this->flags.contains(u"matlab_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(true);
        } else if (this->flags.contains(u"zero_indexing")) {
            this->lmi_importer_ptr->set_matlab_indexing(false);
        }

        // No named parameters... try to interpret inputs as matrix system, depth and word.
        assert(this->inputs.size() == 3); // should be guaranteed by parent.
        this->lmi_importer_ptr->read_level(inputs[1]);


        // Get input type flag
        auto expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence;
        if (this->flags.contains(u"symbols")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::SymbolCell;
        } else if (this->flags.contains(u"operators")) {
            expr_type = LocalizingMatrixIndexImporter::ExpressionType::OperatorCell;
        }

        // Get word
        this->lmi_importer_ptr->read_localizing_expression(inputs[2], expr_type);

        this->parse_optional_params();

    }

    bool LocalizingMatrixParams::any_param_set() const {
        const bool level_specified = this->params.contains(u"level");
        const bool word_specified = this->params.contains(u"word");
        return level_specified || word_specified || OperatorMatrixParams::any_param_set();
    }

    void LocalizingMatrixParams::parse_optional_params() {
        // Get NN if any
        this->find_and_parse(u"neighbours", [this](const matlab::data::Array& nn_param) {
            this->lmi_importer_ptr->read_nearest_neighbour(nn_param);
        });
    }


    LocalizingMatrix::LocalizingMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} {
        // Either [ref, level, word] or named version thereof.
        this->param_names.erase(u"index");
        this->param_names.emplace(u"level");
        this->param_names.emplace(u"word");
        this->param_names.emplace(u"neighbours");

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"operators");
        this->mutex_params.add_mutex(u"symbols", u"operators");

        this->flag_names.emplace(u"zero_indexing");
        this->flag_names.emplace(u"matlab_indexing");
        this->mutex_params.add_mutex(u"zero_indexing", u"matlab_indexing");

        this->max_inputs = 3;
    }

    namespace {
        /** Cast to PauliMatrixSystem, or throw matlab error. */
        [[nodiscard]] Pauli::PauliMatrixSystem& pms_or_throw(matlab::engine::MATLABEngine& matlabEngine,
                                                             MatrixSystem& system) {
            auto* pms_ptr = dynamic_cast<Pauli::PauliMatrixSystem*>(&system);
            if (pms_ptr == nullptr) {
                throw BadParameter{"Nearest neighbours can only be set in Pauli scenario."};
            }
            return *pms_ptr;
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getMonoLM(matlab::engine::MATLABEngine& matlabEngine, MatrixSystem &system,
                  LocalizingMatrixParams& input, Multithreading::MultiThreadPolicy mt_policy) {
            if (input.lmi_importer().has_nn_info()) {
                auto& pauli_system = pms_or_throw(matlabEngine, system);
                auto read_lock = system.get_read_lock();
                auto plmi = input.lmi_importer().to_pauli_monomial_index();
                auto offset = pauli_system.PauliLocalizingMatrices.find_index(plmi);
                if (offset >= 0) {
                    return {offset, system[offset]};
                }

                // Try to create
                read_lock.unlock();
                return pauli_system.PauliLocalizingMatrices.create(plmi, mt_policy);
            } else {
                // Try to get via read-lock only
                auto read_lock = system.get_read_lock();
                auto lmi = input.lmi_importer().to_monomial_index();
                auto offset = system.LocalizingMatrix.find_index(lmi);
                if (offset >= 0) {
                    return {offset, system[offset]};
                }

                // Try to create
                read_lock.unlock();
                return system.LocalizingMatrix.create(lmi, mt_policy);
            }
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolySymbolLMExistingSymbols(
                matlab::engine::MATLABEngine& matlabEngine,
                MaintainsMutex::ReadLock&& symbol_read_lock,
                MatrixSystem &system, LocalizingMatrixParams& input,
                Multithreading::MultiThreadPolicy mt_policy) {

            auto swap_locks = [](MatrixSystem& system, MaintainsMutex::ReadLock symbol_lock)
                    -> MaintainsMutex::ReadLock {
                if (system.is_locked_read_lock(symbol_lock)) {
                    return std::move(symbol_lock);
                }
                return system.get_read_lock(); // ~unlocks symbol lock at this point
            };

            if (input.lmi_importer().has_nn_info()) { // Get Pauli NN index
                auto& pauliMatrixSystem = pms_or_throw(matlabEngine, system);
                auto plmi = input.lmi_importer().to_pauli_polynomial_index();

                // Try to get in read-only mode
                auto matrix_read_lock = swap_locks(system, std::move(symbol_read_lock));
                assert(!symbol_read_lock.owns_lock());
                auto offset = pauliMatrixSystem.PauliPolynomialLocalizingMatrices.find_index(plmi);
                if (offset >= 0) {
                    return {offset, system[offset]}; // ~read_lock
                }

                // Try to create polynomial matrix
                matrix_read_lock.unlock();
                return pauliMatrixSystem.PauliPolynomialLocalizingMatrices.create(std::move(plmi), mt_policy);
            } else {
                // Try to get in read-only mode
                auto plmi = input.lmi_importer().to_polynomial_index();

                auto matrix_read_lock = swap_locks(system, std::move(symbol_read_lock));
                assert(!symbol_read_lock.owns_lock());

                auto offset = system.PolynomialLocalizingMatrix.find_index(plmi);
                if (offset >= 0) {
                    return {offset, system[offset]};// ~read_lock
                }

                // Try to create polynomial matrix
                matrix_read_lock.unlock();
                return system.PolynomialLocalizingMatrix.create(std::move(plmi), mt_policy);
            }
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolySymbolLM(matlab::engine::MATLABEngine& matlabEngine,
                        MatrixSystem &system, LocalizingMatrixParams& input,
                        Multithreading::MultiThreadPolicy mt_policy) {
            auto symbol_read_lock = [&system]() -> MaintainsMutex::ReadLock {
                if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                    return dms_ptr->base_system().get_read_lock();
                }
                return system.get_read_lock();
            }();

            return getPolySymbolLMExistingSymbols(matlabEngine, std::move(symbol_read_lock), system, input, mt_policy);
        }


        std::pair<size_t, const Moment::SymbolicMatrix &>
        getPolyOpLM(matlab::engine::MATLABEngine& matlabEngine,
                    MatrixSystem &system, LocalizingMatrixParams& input,
                    Multithreading::MultiThreadPolicy mt_policy) {
            // Can expression be parsed without new symbols?
            auto symbol_read_lock = [&system]() -> MaintainsMutex::ReadLock {
                if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                    return dms_ptr->base_system().get_read_lock();
                }
                return system.get_read_lock();
            }();

            const bool found_all = input.lmi_importer().attempt_to_find_symbols_from_op_cell(symbol_read_lock);


            // Not all symbols found, so switch to write lock
            if (!found_all) {
                symbol_read_lock.unlock();

                auto symbol_write_lock = [&system]() -> MaintainsMutex::WriteLock {
                    if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                        return dms_ptr->base_system().get_write_lock();
                    }
                    return system.get_write_lock();
                }();

                input.lmi_importer().register_symbols_in_op_cell(symbol_write_lock);

                // Either swap locks, or directly move...
                auto matrix_write_lock = [&]() -> MaintainsMutex::WriteLock {
                    if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                        symbol_write_lock.unlock();
                        return dms_ptr->base_system().get_write_lock();
                    }
                    return std::move(symbol_write_lock);
                }();
                assert(!symbol_write_lock.owns_lock()); // ~ above should have unlocked...

                // And invoke
                if (input.lmi_importer().has_nn_info()) {
                    auto& pauliMatrixSystem = pms_or_throw(matlabEngine, system);
                    auto index = input.lmi_importer().to_pauli_polynomial_index();
                    return pauliMatrixSystem.PauliPolynomialLocalizingMatrices.create(matrix_write_lock, std::move(index),
                                                                                      mt_policy);
                } else {
                    auto index = input.lmi_importer().to_polynomial_index();
                    return system.PolynomialLocalizingMatrix.create(matrix_write_lock, index, mt_policy);
                }
            }

            // Fall-back to normal
            return getPolySymbolLMExistingSymbols(matlabEngine, std::move(symbol_read_lock), system, input, mt_policy);
        }

        std::pair<size_t, const Moment::SymbolicMatrix &>
        getAliasedPolyOpLM(matlab::engine::MATLABEngine& matlabEngine,
                           MatrixSystem& system, LocalizingMatrixParams& input,
                           Multithreading::MultiThreadPolicy mt_policy) {

            // Must be able to parse expression without registering new symbols
            auto symbol_read_lock = [&system]() -> MaintainsMutex::ReadLock {
                if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                    return dms_ptr->base_system().get_read_lock();
                }
                return system.get_read_lock();
            }();

            // Apply context to
            auto& lmi_importer = input.lmi_importer();
            lmi_importer.supply_context_only(symbol_read_lock);

            if (auto * pms_ptr = dynamic_cast<Pauli::PauliMatrixSystem*>(&system); pms_ptr != nullptr) {
                Pauli::PauliMatrixSystem& pauli_system = *pms_ptr;
                auto [raw_level, raw_poly] = lmi_importer.to_pauli_raw_polynomial_index();
                symbol_read_lock.unlock(); // symbol read finished, and next line calls write lock.
                return pauli_system.create_and_register_localizing_matrix(raw_level, raw_poly, mt_policy);
            } else {
                auto [raw_level, raw_poly] = lmi_importer.to_raw_polynomial_index();
                symbol_read_lock.unlock(); // symbol read finished, and next line calls write lock.
                return system.create_and_register_localizing_matrix(raw_level, raw_poly, mt_policy);
            }
        }
    }

    std::pair<size_t, const Moment::SymbolicMatrix &>
    LocalizingMatrix::get_or_make_matrix(MatrixSystem &system, OperatorMatrixParams &inputOMP) {
        auto &input = dynamic_cast<LocalizingMatrixParams&>(inputOMP);
        // Attach matrix system to index reader
        input.lmi_importer().link_matrix_system(&system);

        // Check if index could be aliased in some way
        const bool can_have_aliases = [&system]() {
            if (const auto* dms_ptr = dynamic_cast<const Derived::DerivedMatrixSystem*>(&system); dms_ptr != nullptr) {
                return dms_ptr->base_system().Context().can_have_aliases();
            } else {
                return system.Context().can_have_aliases();
            }
        }();

        // Switch based on type
        try {
            switch (input.lmi_importer().get_expression_type()) {
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorSequence:
                    return getMonoLM(matlabEngine, system, input, this->settings->get_mt_policy());
                case LocalizingMatrixIndexImporter::ExpressionType::SymbolCell:
                    if (!this->quiet && can_have_aliases) {
                        print_warning(matlabEngine,
                              "When a scenario has aliases (e.g. due to symmetry), symbol cell input might produce unexpected results:\n"
                              "The input Polynomial will be symmetrized before the localizing matrices!");
                    }
                    return getPolySymbolLM(matlabEngine, system, input, this->settings->get_mt_policy());
                case LocalizingMatrixIndexImporter::ExpressionType::OperatorCell:
                    if (can_have_aliases) {
                        return getAliasedPolyOpLM(matlabEngine, system, input, this->settings->get_mt_policy());
                    } else {
                        return getPolyOpLM(matlabEngine, system, input, this->settings->get_mt_policy());
                    }
                default:
                case LocalizingMatrixIndexImporter::ExpressionType::Unknown:
                    throw InternalError{"Unknown localizing expression type."};
            }
        } catch (std::exception& e) {
            throw InternalError{std::string("A problem occurred while retrieving/generating localizing matrix: ")
                                + e.what()};
        }

    }
}