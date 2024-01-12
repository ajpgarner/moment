/**
 * binary_operation.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "binary_operation.h"

#include <ostream>

namespace Moment::mex::functions {

    std::ostream& operator<<(std::ostream& os, ProductType pt) {
        switch (pt) {
            case ProductType::Incompatible:
                os << "Incompatible";
                break;
            case ProductType::MismatchedDimensions:
                os << "MismatchedDimensions";
                break;
            case ProductType::OneToOne:
                os << "OneToOne";
                break;
            case ProductType::OneToMany:
                os << "OneToMany";
                break;
            case ProductType::ManyToOne:
                os << "ManyToOne";
                break;
            case ProductType::ManyToMany:
                os << "ManyToMany";
                break;
            case ProductType::OneToMatrix:
                os << "OneToMatrix";
                break;
            case ProductType::MatrixToOne:
                os << "MatrixToOne";
                break;
            case ProductType::MatrixToMatrix:
                os << "MatrixToMatrix";
                break;
            default:
                os << "[Unknown: " << static_cast<uint64_t>(pt) << "]";
                break;
        }

        return os;
    }

    ProductType determine_product_type(const AlgebraicOperand& lhs, const AlgebraicOperand& rhs) noexcept {
        switch (lhs.type) {
            // Not a case! Deliberate: this is an instruction to the compiler, not executable code...
            using enum AlgebraicOperand::InputType;

            // Matrix ->
            case MatrixID:
                switch (rhs.type) {
                    // -> unknown
                    case Unknown:
                        return ProductType::Incompatible;
                        // -> one
                    case EmptyObject:
                    case Monomial:
                    case Polynomial:
                        return ProductType::MatrixToOne;
                        // -> matrix
                    case MatrixID:
                        return ProductType::MatrixToMatrix;
                        // -> many
                    case MonomialArray:
                    case PolynomialArray:
                        return ProductType::Incompatible;
                }
                break;

                // One ->
            case EmptyObject:
            case Monomial:
            case Polynomial:
                switch (rhs.type) {
                    // -> unknown
                    case Unknown:
                        return ProductType::Incompatible;
                        // -> one
                    case EmptyObject:
                    case Monomial:
                    case Polynomial:
                        return ProductType::OneToOne;
                        // -> matrix
                    case MatrixID:
                        return ProductType::OneToMatrix;
                        // -> many
                    case MonomialArray:
                    case PolynomialArray:
                        return ProductType::OneToMany;
                }
                break;

                // Many ->
            case PolynomialArray:
            case MonomialArray:
                switch (rhs.type) {
                    // -> unknown
                    case Unknown:
                        return ProductType::Incompatible;
                        // -> one
                    case EmptyObject:
                    case Monomial:
                    case Polynomial:
                        return ProductType::ManyToOne;
                        // -> matrix
                    case MatrixID:
                        return ProductType::Incompatible;
                        // -> many
                    case MonomialArray:
                    case PolynomialArray:
                        if (!std::equal(lhs.shape.cbegin(), lhs.shape.cend(),
                                        rhs.shape.cbegin(), rhs.shape.cend())) {
                            return ProductType::MismatchedDimensions;
                        }
                        return ProductType::ManyToMany;
                }
                break;

                // Unknown: cannot combine
            default:
            case Unknown:
                return ProductType::Incompatible;
        }

        // Unreachable~
        return ProductType::Incompatible;
    }

    BinaryOperationParams::BinaryOperationParams(SortedInputs&& raw_inputs)
        : SortedInputs{std::move(raw_inputs)}, matrix_system_key{matlabEngine},
          lhs{matlabEngine, "LHS"}, rhs{matlabEngine, "RHS"} {

        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Check type of LHS input
        this->lhs.parse_input(this->inputs[1]);

        // Check type of RHS input
        this->rhs.parse_input(this->inputs[2]);

        // Check dimensions / type
        this->resolved_product_type = determine_product_type(this->lhs, this->rhs);
    }

    std::string BinaryOperationParams::to_string() const {
        std::stringstream ss;
        ss << "Binary operation.\n"
           << "System:\t 0x" << std::hex << this->matrix_system_key.value() << std::dec << "\n"
           << "LHS:\t" << this->lhs << "\n"
           << "RHS:\t" << this->rhs << "\n"
           << "Product: " << this->resolved_product_type;
        return ss.str();
    }
}