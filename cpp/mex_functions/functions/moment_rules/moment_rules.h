/**
 * moment_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "mex_function.h"
#include "integer_types.h"

#include "import/read_polynomial.h"

#include <map>
#include <memory>

namespace Moment {
    class MatrixSystem;
    class MomentRulebook;
    class Polynomial;
    class PolynomialFactory;
    class SymbolTable;
}

namespace Moment::mex {
    class StagingPolynomial;
}

namespace Moment::mex::functions {

    class MomentRulesParams : public SortedInputs {
    public:
        /** The matrix system the ruleset is associated with */
        uint64_t matrix_system_key = 0;

        /** The ID of an existing ruleset  */
        uint64_t rulebook_index = 0;

        /** How should we output rules */
        enum class OutputMode {
            /** List rules as string array. */
            String,
            /** List rules as symbolic cell array. */
            SymbolCell,
            /** List rules as a cell array of polynomials. */
            Polynomial,
            /** List rules as a cell array of polynomials. */
            PolynomialWithSymbolInfo,
            /** List rules as sparse matrix that can act on an arbitrary (a \oplus b) vector to fill missing values. */
            RewriteMatrix,
            /** List rules as sparse matrix that can act on conformant (a \oplus b) vector to equal zero. */
            HomogenousMatrix,
        } output_mode = OutputMode::String;

        /** Constructor */
        explicit MomentRulesParams(SortedInputs &&rawInput);
    };

    class MomentRules : public ParameterizedMexFunction<MomentRulesParams, MEXEntryPointID::MomentRules> {
    public:
        explicit MomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage);

        void extra_input_checks(MomentRulesParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, MomentRulesParams &input) override;
    };
}