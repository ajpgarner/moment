/**
 * localizing_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"

#include "import/read_polynomial.h"

#include "matrix_system/localizing_matrix_index.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"

#include <variant>

namespace Moment {
    class Context;
    namespace mex {
        class StagingPolynomial;
        class LocalizingMatrixIndexImporter;
    }
}

namespace Moment::mex::functions  {

    struct LocalizingMatrixParams : public OperatorMatrixParams {
    private:
        std::unique_ptr<LocalizingMatrixIndexImporter> lmi_importer_ptr;
    public:
        explicit LocalizingMatrixParams(SortedInputs&& inputs);

        ~LocalizingMatrixParams() noexcept;

        LocalizingMatrixIndexImporter& lmi_importer() noexcept { return *lmi_importer_ptr; }

        const LocalizingMatrixIndexImporter& lmi_importer() const noexcept { return *lmi_importer_ptr; }


    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        void parse_optional_params();

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level, word]"; }

    private:

    };

    class LocalizingMatrix
        : public Moment::mex::functions::OperatorMatrix<LocalizingMatrixParams, MTKEntryPointID::LocalizingMatrix> {
    public:
        explicit LocalizingMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}
