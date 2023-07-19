/**
 * transform_symbols.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../mex_function.h"

#include "integer_types.h"
#include "import/read_polynomial.h"

#include <variant>
#include <vector>

namespace Moment::Derived {
    class DerivedMatrixSystem;
}

namespace Moment::mex::functions {

    struct TransformSymbolsParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        enum class InputType {
            Unknown,
            SymbolId,
            SymbolCell,
            Basis
        } input_type = InputType::Unknown;

        enum class OutputType {
            Unknown,
            String,
            SymbolCell,
            Basis
        } output_type = OutputType::Unknown;


        std::variant<std::vector<symbol_name_t>,
                     std::vector<std::vector<raw_sc_data>>> input_data;

        std::vector<size_t> input_shape;

    public:
        explicit TransformSymbolsParams(SortedInputs&& structuredInputs);

        [[nodiscard]] constexpr std::vector<symbol_name_t>& symbol_id() {
            return std::get<0>(input_data);
        }

        [[nodiscard]] constexpr std::vector<std::vector<raw_sc_data>>& raw_polynomials() {
            return std::get<1>(input_data);
        }

    private:
        void read_symbol_ids(matlab::data::Array& input);

        void read_symbol_cell(matlab::data::Array& input);

        void read_basis(matlab::data::Array& real, matlab::data::Array& imaginary);


    };

    class TransformSymbols : public ParameterizedMexFunction<TransformSymbolsParams, MEXEntryPointID::TransformSymbols> {
    public:
        explicit TransformSymbols(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage);

    protected:
        void operator()(IOArgumentRange output, TransformSymbolsParams &input) override;

        void extra_input_checks(TransformSymbolsParams &input) const override;

    private:
        void transform_symbol_ids(IOArgumentRange& output, TransformSymbolsParams &input,
                                  const Derived::DerivedMatrixSystem& targetSystem);

        void transform_symbol_cells(IOArgumentRange& output, TransformSymbolsParams &input,
                                    const Derived::DerivedMatrixSystem& targetSystem);

        void transform_basis(IOArgumentRange& output, TransformSymbolsParams &input,
                             const Derived::DerivedMatrixSystem& targetSystem);

    };
}