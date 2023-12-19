/**
 * generate_basis.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../mtk_function.h"

#include "import/matrix_system_id.h"
#include "import/read_polynomial.h"
#include <variant>

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions {

    struct GenerateBasisParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** True, if output should be a sparse matrix */
        bool sparse_output = false;

        /** True, if output should be an indexed sparse array, or a flattened monolithic array */
        bool monolithic_output = false;

        /** The type of input */
        enum class InputType {
            MatrixId,
            SymbolCell
        } input_type = InputType::MatrixId;

    private:
        std::variant<uint64_t, std::vector<std::vector<raw_sc_data>>> input_data;

    public:

        constexpr uint64_t matrix_index() const noexcept {
            return std::get<0>(this->input_data);
        }

        constexpr const std::vector<std::vector<raw_sc_data>>& raw_polynomials() const noexcept {
            return std::get<1>(this->input_data);
        }

        std::vector<size_t> input_shape;

    public:
        explicit GenerateBasisParams(SortedInputs&& structuredInputs);

    private:
        void read_symbol_cell(matlab::data::Array& raw_input);

    };

class GenerateBasis : public ParameterizedMTKFunction<GenerateBasisParams, MTKEntryPointID::GenerateBasis> {
    public:
        explicit GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, GenerateBasisParams &input) override;

    private:
        void generate_matrix_basis(IOArgumentRange& output, GenerateBasisParams &input,
                                   const MatrixSystem& matrixSystem);

        void generate_symbol_cell_basis(IOArgumentRange& output, GenerateBasisParams &input,
                                        const MatrixSystem& matrixSystem);

};

}