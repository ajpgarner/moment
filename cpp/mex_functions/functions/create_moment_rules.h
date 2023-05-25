/**
 * create_moment_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once


#include "../mex_function.h"
#include "integer_types.h"

#include "import/read_symbol_combo.h"

#include <map>
#include <memory>


namespace Moment::Algebraic {
    class NameTable;
    class AlgebraicPrecontext;
}

namespace Moment::mex::functions {

    class CreateMomentRulesParams : public SortedInputs {
    public:
        /** The matrix system the ruleset is associated with */
        uint64_t matrix_system_key = 0;

        /** True = Also generate extra rules from currently-known factors. */
        bool infer_from_factors = true;

        /** How the input to the create-rules command is supplied */
        enum class InputMode {
            Unknown,
            /** List of symbol ID / double value scalar substitutions */
            SubstitutionList,
            /** Polynomials, expressed as symbols. */
            FromSymbolIds,
            /** Polynomials, expressed as operator sequences. */
            FromOperatorSequences
        } input_mode = InputMode::SubstitutionList;

        /** How should symbol IDs be ordered */
        enum class SymbolOrdering {
            Unknown,
            /** Sort symbols by their ID (i.e. order of creation) */
            ById,
            /** Sort symbols by hash of their associated operators */
            ByOperatorHash
        } ordering = SymbolOrdering::ById;

        /** Direct substitutions, if specified. */
        std::map<symbol_name_t, std::complex<double>> sub_list;

        /** Direct set of symbol combos, if specified. */
        std::vector<std::vector<raw_sc_data>> raw_symbol_polynomials;

    public:
        /** Constructor */
        explicit CreateMomentRulesParams(SortedInputs &&rawInput);

        /** Destructor */
        ~CreateMomentRulesParams() noexcept;

    private:
        void parse_as_sublist(const matlab::data::Array& data);
        void parse_as_symbol_polynomials(const matlab::data::Array& data);

    };

    class CreateMomentRules : public ParameterizedMexFunction<CreateMomentRulesParams,
                                                              MEXEntryPointID::CreateMomentRules> {
    public:
        explicit CreateMomentRules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void extra_input_checks(CreateMomentRulesParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, CreateMomentRulesParams &input) override;




    };

}