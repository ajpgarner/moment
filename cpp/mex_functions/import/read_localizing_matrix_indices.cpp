/**
 * read_localizing_matrix_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_localizing_matrix_indices.h"
#include "read_opseq_polynomial.h"

#include "scenarios/context.h"
#include "scenarios/derived/derived_matrix_system.h"

#include "utilities/read_as_scalar.h"

#include "utilities/reporting.h"
#include "errors.h"
#include "utilities/read_as_vector.h"

namespace Moment::mex {

    size_t LocalizingMatrixIndexImporter::read_level(const matlab::data::Array& level_param) {
        this->hierarchy_level = read_positive_integer<size_t>(matlabEngine, "Hierarchy depth", level_param, 0);
        return this->hierarchy_level;
    }

    size_t LocalizingMatrixIndexImporter::read_nearest_neighbour(const matlab::data::Array& nn_param) {
        this->nearest_neighbour = read_positive_integer<size_t>(matlabEngine, "Nearest neighbour count", nn_param, 0);
        return this->nearest_neighbour;
    }

    void LocalizingMatrixIndexImporter::read_localizing_expression(const matlab::data::Array& expr,
                                                                   ExpressionType expr_type) {
        // Get expression type
        this->expression_type = expr_type;

        // Handle input based on MATLAB type:
        switch (expr.getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                // Check type is consistent
                if (ExpressionType::Unknown == this->expression_type) {
                    this->expression_type = ExpressionType::OperatorSequence;
                } else if (ExpressionType::OperatorSequence != this->expression_type) {
                    throw BadParameter{"Cell input specified, but operator sequence supplied."};
                }
                // Read operator sequence
                this->localizing_expression.emplace<0>(read_integer_array<oper_name_t>(matlabEngine,
                                                                                       "Localizing expression", expr));
                // Do MATLAB offset:
                if (this->matlab_indexing) {
                    auto& raw_expr = std::get<0>(this->localizing_expression);
                    for (auto& o: raw_expr) {
                        if (0 == o) {
                            throw BadParameter{"Operator with index 0 in localizing word is out of range."};
                        }
                        o -= 1;
                    }
                }


               break;
            case matlab::data::ArrayType::CELL:
                // Check type is consistent
                if (ExpressionType::Unknown == this->expression_type) {
                    this->expression_type = ExpressionType::OperatorCell;
                } else if (ExpressionType::OperatorSequence == this->expression_type) {
                    throw BadParameter{"Operator sequence specified, but cell array supplied."};
                }
                if (ExpressionType::SymbolCell == this->expression_type) {
                    this->localizing_expression.emplace<1>(
                            read_raw_polynomial_data(this->matlabEngine, "Localizing expression", expr));
                } else {
                    this->localizing_expression.emplace<2>(
                            std::make_unique<StagingPolynomial>(this->matlabEngine, expr, "Localizing expression"));
                }
                break;
            default:
            case matlab::data::ArrayType::UNKNOWN:
                throw BadParameter{"Localizing expression must be an operator sequence, or a polynomial cell definition."};
        }
    }

    bool LocalizingMatrixIndexImporter::attempt_to_find_symbols_from_op_cell(const MaintainsMutex::ReadLock& rlock) {

        // Check mode
        if constexpr (debug_mode) {
            if (this->matrix_system == nullptr) {
                throw InternalError{"MatrixSystem not linked."};
            }
            if (ExpressionType::OperatorCell != this->expression_type) {
                throw InternalError{"No operator cell is defined by this LocalizingMatrixIndex."};
            }
        }

        auto& staging_poly = *std::get<2>(this->localizing_expression);

        // Check if derived...
        if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(this->matrix_system); dms_ptr != nullptr) {
            if constexpr (debug_mode) {
                if (!dms_ptr->base_system().is_locked_read_lock(rlock)) {
                    throw InternalError{"Incorrect read lock held for symbol read (expected base system lock)."};
                }
            }
            staging_poly.supply_context(dms_ptr->base_system().Context());
            return staging_poly.find_symbols(dms_ptr->base_system().Symbols(), true);
        }

        if constexpr (debug_mode) {
            if (!this->matrix_system->is_locked_read_lock(rlock)) {
                throw InternalError{"Incorrect read lock held for symbol read."};
            }
        }
        staging_poly.supply_context(this->matrix_system->Context());
        return staging_poly.find_symbols(this->matrix_system->Symbols(), true);
    }

    void LocalizingMatrixIndexImporter::supply_context_only(const MaintainsMutex::ReadLock& rlock) {

        // Check mode
        if constexpr (debug_mode) {
            if (this->matrix_system == nullptr) {
                throw InternalError{"MatrixSystem not linked."};
            }
            if (ExpressionType::OperatorCell != this->expression_type) {
                throw InternalError{"No operator cell is defined by this LocalizingMatrixIndex."};
            }
        }

        auto& staging_poly = *std::get<2>(this->localizing_expression);

        // Check if derived...
        if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(this->matrix_system); dms_ptr != nullptr) {
            if constexpr (debug_mode) {
                if (!dms_ptr->base_system().is_locked_read_lock(rlock)) {
                    throw InternalError{"Incorrect read lock held for symbol read (expected base system lock)."};
                }
            }
            staging_poly.supply_context(dms_ptr->base_system().Context());
        } else {
            if constexpr (debug_mode) {
                if (!this->matrix_system->is_locked_read_lock(rlock)) {
                    throw InternalError{"Incorrect read lock held for symbol read."};
                }
            }

            staging_poly.supply_context(this->matrix_system->Context());
        }
    }

    void LocalizingMatrixIndexImporter::register_symbols_in_op_cell(const MaintainsMutex::WriteLock& wlock) {
        auto& staging_poly = *std::get<2>(this->localizing_expression);
        // Check if derived...
        if (auto dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(this->matrix_system); dms_ptr != nullptr) {
            if constexpr (debug_mode) {
                if (!dms_ptr->base_system().is_locked_write_lock(wlock)) {
                    throw InternalError{"Incorrect write lock held for symbol write (expected base system lock)."};
                }
            }
            staging_poly.find_or_register_symbols(dms_ptr->base_system().Symbols());
            return;
        }
        if constexpr (debug_mode) {
            if (!this->matrix_system->is_locked_write_lock(wlock)) {
                throw InternalError{"Incorrect write lock held for symbol write."};
            }
        }
        staging_poly.find_or_register_symbols(this->matrix_system->Symbols());
    }


    LocalizingMatrixIndex LocalizingMatrixIndexImporter::to_monomial_index() const {

        // Check mode
        if constexpr (debug_mode) {
            if (this->matrix_system == nullptr) {
                throw InternalError{"MatrixSystem not linked."};
            }
            if (ExpressionType::OperatorSequence != this->expression_type) {
                throw InternalError{"No monomial is defined by this LocalizingMatrixIndex."};
            }
        }

        const auto& localizing_word_raw = std::get<0>(this->localizing_expression);

        // Get context from this, or derived system
        const auto& context = [this]() -> const Context& {
            if (auto* dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(this->matrix_system);
                dms_ptr != nullptr) {
                return dms_ptr->base_system().Context();
            }
            return this->matrix_system->Context();
        }();

        // Do we have to offset?
        sequence_storage_t oper_copy{};
        oper_copy.reserve(localizing_word_raw.size());
        for (auto o : localizing_word_raw) {
            // Check in range
            if ((o < 0) || (o >= context.size())) {
                std::stringstream errSS;
                errSS << "Operator " << (this->matlab_indexing ? o + 1 : o) << " at index ";
                errSS << (oper_copy.size() + 1);
                errSS << " is out of range.";
                throw BadParameter{errSS.str()};
            }
            oper_copy.emplace_back(o);
        }

        // Copy and construct LMI
        return LocalizingMatrixIndex{this->hierarchy_level, OperatorSequence{std::move(oper_copy), context}};
    }


    ::Moment::PolynomialLocalizingMatrixIndex LocalizingMatrixIndexImporter::to_polynomial_index() const {
        // Check
        if constexpr (debug_mode) {
            if (this->matrix_system == nullptr) {
                throw InternalError{"MatrixSystem not linked."};
            }
        }

        // Get factory from this, or derived system
        const auto& factory = [&]() -> const PolynomialFactory& {
            if (auto* dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(this->matrix_system);
                    dms_ptr != nullptr) {
                return dms_ptr->base_system().polynomial_factory();
            }
            return this->matrix_system->polynomial_factory();
        }();

        switch (this->expression_type) {
            case ExpressionType::SymbolCell:
                return ::Moment::PolynomialLocalizingMatrixIndex{this->hierarchy_level,
                                         raw_data_to_polynomial(this->matlabEngine, factory,
                                                                std::get<1>(this->localizing_expression))};
            case ExpressionType::OperatorCell: {
                const auto& staging_poly = *std::get<2>(this->localizing_expression);
                if (!staging_poly.ready()) {
                    throw InternalError{"OperatorCell polynomial has not yet been resolved into symbols."};
                }
                return ::Moment::PolynomialLocalizingMatrixIndex{this->hierarchy_level, staging_poly.to_polynomial(factory)};
            }
            default:
                throw InternalError{"Localizing expression was not given as symbol cell array."};
        }
    }


    std::pair<size_t, RawPolynomial>
    LocalizingMatrixIndexImporter::to_raw_polynomial_index() const {

        if (this->expression_type != ExpressionType::OperatorCell) {
            throw InternalError{"RawPolynomial only results from operator cell input."};
        }

        const auto& staging_poly = *std::get<2>(this->localizing_expression);
        return std::pair<size_t, RawPolynomial>{this->hierarchy_level, staging_poly.to_raw_polynomial()};
    }

    Pauli::LocalizingMatrixIndex LocalizingMatrixIndexImporter::to_pauli_monomial_index() const {
        // Read and check normal LMI expression
        auto lmi = this->to_monomial_index();

        // Inject NN and wrap info
        return Pauli::LocalizingMatrixIndex{Pauli::NearestNeighbourIndex{lmi.Level, this->nearest_neighbour},
                                            std::move(lmi.Word)};

    }

    Pauli::PolynomialLocalizingMatrixIndex LocalizingMatrixIndexImporter::to_pauli_polynomial_index() const {
        // Read and check normal LMI expression
        auto lmi = this->to_polynomial_index();

        // Inject NN and wrap info
        return Pauli::PolynomialLocalizingMatrixIndex{Pauli::NearestNeighbourIndex{lmi.Level, this->nearest_neighbour},
                                                      std::move(lmi.Polynomial)};
    }

    std::pair<Pauli::NearestNeighbourIndex, RawPolynomial>
    LocalizingMatrixIndexImporter::to_pauli_raw_polynomial_index() const {
        using namespace Pauli;
        if (this->expression_type != ExpressionType::OperatorCell) {
            throw InternalError{"RawPolynomial only results from operator cell input."};
        }

        const auto& staging_poly = *std::get<2>(this->localizing_expression);
        return std::pair<Pauli::NearestNeighbourIndex, RawPolynomial>{
            NearestNeighbourIndex{this->hierarchy_level, this->nearest_neighbour}, staging_poly.to_raw_polynomial()
        };
    }
}