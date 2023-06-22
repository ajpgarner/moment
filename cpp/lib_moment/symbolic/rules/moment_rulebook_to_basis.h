/**
 * moment_rulebook_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "Eigen/SparseCore"

namespace Moment {
    class SymbolTable;
    class MomentRulebook;
    class PolynomialFactory;

    /**
     * Convert a Polynomial into a vector of basis co-efficients.
     * Underlying MatrixSystem should be read-locked before invoking.
     */
    class MomentRulebookToBasis {
    public:
        using output_t = Eigen::SparseMatrix<double>;

        const SymbolTable& symbols;
        const double zero_tolerance;
        const enum struct ExportMode {
            /** Export matrix M such that a'\oplus b' = M (a\oplus b). */
            Rewrite,
            /** Export matrix N such that N(a\oplus b) == 0. */
            Homogeneous
        } export_mode;

    public:
        explicit MomentRulebookToBasis(const PolynomialFactory& factory, ExportMode mode = ExportMode::Rewrite);

        explicit MomentRulebookToBasis(const SymbolTable& symbols, double zero_tolerance,
                                       ExportMode mode = ExportMode::Rewrite);

        output_t operator()(const MomentRulebook& rulebook) const;
    };

}