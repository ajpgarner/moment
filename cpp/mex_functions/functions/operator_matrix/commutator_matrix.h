/**
 * extended_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"
#include "integer_types.h"

#include <memory>

namespace Moment::mex {
    class LocalizingMatrixIndexImporter;
}

namespace Moment::mex::functions {

    class CommutatorMatrixParams : public OperatorMatrixParams {
    public:
        size_t hierarchy_level = 0;
        size_t nearest_neighbours = 0;

        enum class RequestedMatrix {
            Commutator,
            Anticommutator
        } requested_matrix = RequestedMatrix::Commutator;


    private:
        std::unique_ptr<LocalizingMatrixIndexImporter> lmi_importer_ptr;
    public:

        explicit CommutatorMatrixParams(SortedInputs&& input);

        ~CommutatorMatrixParams() noexcept;

        LocalizingMatrixIndexImporter& lmi_importer() noexcept { return *lmi_importer_ptr; }

        const LocalizingMatrixIndexImporter& lmi_importer() const noexcept { return *lmi_importer_ptr; }

        void parse_optional_params();

        void extra_parse_params() final;

        void extra_parse_inputs() final;

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level, word]"; }

    private:

    };

    class CommutatorMatrix
        : public Moment::mex::functions::OperatorMatrix<CommutatorMatrixParams, MTKEntryPointID::CommutatorMatrix> {
    public:
        explicit CommutatorMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) override;

    };
}