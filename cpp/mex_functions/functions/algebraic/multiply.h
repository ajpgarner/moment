/**
 * multiply.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */


#include "../../mtk_function.h"
#include "integer_types.h"
#include "import/read_polynomial.h"

#include <span>
#include <string>
#include <variant>

namespace Moment {
    class Context;
    class MatrixSystem;
}

namespace Moment::mex::functions  {

    struct MultiplyParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        struct Operand {
            enum class InputType {
                Unknown,
                MatrixID,
                Polynomial,
                PolynomialArray
            } type = InputType::Unknown;

            std::vector<size_t> shape;
            std::variant<size_t, std::vector<std::vector<raw_sc_data>>> raw;

        public:
            [[nodiscard]] size_t matrix_key() const { return std::get<0>(this->raw); }

            [[nodiscard]] std::vector<std::vector<raw_sc_data>>& raw_polynomials() {
                return std::get<1>(this->raw);
            }

            [[nodiscard]] const std::vector<std::vector<raw_sc_data>>& raw_polynomials() const {
                return std::get<1>(this->raw);
            }

        public:
            Operand() = default;
            Operand(const Operand& lhs) = delete;
            Operand(Operand&& lhs) = default;
            Operand& operator=(const Operand& rhs) = delete;
            Operand& operator=(Operand&& rhs) = default;
        };

        Operand lhs;
        Operand rhs;

    private:
        [[nodiscard]] Operand parse_as_matrix_key(const std::string& name, matlab::data::Array& input);

        [[nodiscard]] Operand parse_as_polynomial(const std::string& name, matlab::data::Array& input);

    public:
        explicit MultiplyParams(SortedInputs&& inputs);

        enum class OutputMode {
            Unknown,
            MatrixIndex,
            String,
            SymbolCell,
            SequencesWithSymbolInfo
        } output_mode = OutputMode::MatrixIndex;
    };

    class Multiply : public ParameterizedMTKFunction<MultiplyParams, MTKEntryPointID::Multiply> {
    public:
        explicit Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MultiplyParams &input) override;

        void extra_input_checks(MultiplyParams &input) const override;

    private:
        void matrix_by_polynomial(IOArgumentRange& output, const MultiplyParams& input, MatrixSystem& system);

    };
}
