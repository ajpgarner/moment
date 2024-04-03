/**
 * import_polynomial.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../mtk_function.h"

#include "import/matrix_system_id.h"
#include "import/read_polynomial.h"

#include "integer_types.h"


#include <string>

namespace Moment::mex::functions  {

    struct ImportPolynomialParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** Set to true, to register new symbols in table (by default, as complex). */
        bool register_new = false;

        /** The data as (raw) polynomials */
        std::vector<std::vector<raw_sc_data>> inputPolynomials;

        /** Dimensions of the polynomial array */
        std::vector<size_t> input_shape;

        /** How should the simplified polynomial be output. */
        enum class OutputType {
            SymbolCell,
            String
        } output_type = OutputType::SymbolCell;

    public:
        explicit ImportPolynomialParams(SortedInputs&& inputs);

    private:


    };

    class ImportPolynomial : public ParameterizedMTKFunction<ImportPolynomialParams,
                                                             MTKEntryPointID::ImportPolynomial> {
    public:
        ImportPolynomial(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ImportPolynomialParams &input) override;
    };
}