/**
 * substituted_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "./operator_matrix.h"

#include "integer_types.h"

#include <string>

namespace Moment::mex::functions  {

    struct SubstitutedMatrixParams : public OperatorMatrixParams {
    public:
        uint64_t matrix_index = 0;
        uint64_t rules_index = 0;

    public:
        explicit SubstitutedMatrixParams(SortedInputs&& inputs) : OperatorMatrixParams(std::move(inputs)) { }

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        /** True if reference id, or derived parameter (e.g. level, word, etc.), set */
        [[nodiscard]] bool any_param_set() const final;

        /** Number of inputs required to fully specify matrix requested */
        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        /** Correct format */
        [[nodiscard]] std::string input_format() const final {
            return "[matrix system ID, matrix index, rulebook index]";
        }
    };

    class SubstitutedMatrix : public Moment::mex::functions::OperatorMatrix<SubstitutedMatrixParams,
                                                                            MEXEntryPointID::SubstitutedMatrix> {
    public:
        SubstitutedMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::Matrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}