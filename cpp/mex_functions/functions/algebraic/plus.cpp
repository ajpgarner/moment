/**
 * plus.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "plus.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include "storage_manager.h"

#include "scenarios/context.h"
#include "export/export_polynomial.h"

namespace Moment::mex::functions {

    PlusParams::PlusParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(this->matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Get left operand
        this->lhs = this->parse_as_polynomial("LHS", this->inputs[1]);

        // Get right operand
        this->rhs = this->parse_as_polynomial("RHS", this->inputs[2]);

        // Check if we have to broadcast
        if (this->lhs.raw.size() == 1) {
            this->distribution_mode = (this->rhs.raw.size() == 1)
                                    ? DistributionMode::ManyToMany : DistributionMode::OneToMany;
        } else {
            this->distribution_mode = (this->rhs.raw.size() == 1)
                                      ? DistributionMode::ManyToOne : DistributionMode::ManyToMany;
        }

        // Check sizes
        if (this->distribution_mode == DistributionMode::ManyToMany) {
            if (!std::equal(this->lhs.shape.cbegin(), this->lhs.shape.cend(),
                            this->rhs.shape.cbegin(), this->rhs.shape.cend())) {
                throw_error(this->matlabEngine, errors::bad_param,
                            "Argument dimensions must match (or one element must be a scalar) to use plus.");
            }
        }

        // Calculate output size
        switch (this->distribution_mode) {
            case DistributionMode::ManyToMany:
            case DistributionMode::ManyToOne:
                this->output_shape.reserve(this->lhs.shape.size());
                std::copy(this->lhs.shape.cbegin(), this->lhs.shape.cend(), std::back_inserter(this->output_shape));
                this->output_size = this->lhs.raw.size();
                break;
            case DistributionMode::OneToMany:
                this->output_shape.reserve(this->rhs.shape.size());
                std::copy(this->rhs.shape.cbegin(), this->rhs.shape.cend(), std::back_inserter(this->output_shape));
                this->output_size = this->rhs.raw.size();
                break;
        }

        // How do we output?
        if (this->flags.contains(u"strings")) {
            this->output_mode = OutputMode::String;
        } else if (this->flags.contains(u"sequences")) {
            this->output_mode = OutputMode::SequencesWithSymbolInfo;
        } else {
            this->output_mode = OutputMode::SymbolCell;
        }
    }

    PlusParams::Operand PlusParams::parse_as_polynomial(const std::string& name, matlab::data::Array& raw_input) {
        Operand raw;
        raw.type = Operand::InputType::SymbolCell;
        const auto dimensions = raw_input.getDimensions();
        raw.shape.reserve(dimensions.size());
        std::copy(dimensions.cbegin(), dimensions.cend(), std::back_inserter(raw.shape));

        if (raw_input.getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Polynomial mode expects symbol cell input.");
        }


        raw.raw.reserve(inputs[1].getNumberOfElements());

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = raw_input;
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            raw.raw.emplace_back(read_raw_polynomial_data(this->matlabEngine, name, *read_iter));
            ++read_iter;
        }
        return raw;
    }

    void Plus::extra_input_checks(PlusParams& input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Plus::Plus(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"strings"});

    }

    void Plus::operator()(IOArgumentRange output, PlusParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto read_lock = matrixSystem.get_read_lock();
        const auto& poly_factory = matrixSystem.polynomial_factory();

        // Read in LHS
        std::vector<Polynomial> lhs_poly;
        lhs_poly.reserve(input.lhs.raw.size());
        for (auto& raw_lhs : input.lhs.raw) {
            lhs_poly.emplace_back(raw_data_to_polynomial_assume_sorted(this->matlabEngine, poly_factory, raw_lhs));
        }

        // Read in RHS
        std::vector<Polynomial> rhs_poly;
        rhs_poly.reserve(input.rhs.raw.size());
        for (auto& raw_rhs : input.rhs.raw) {
            rhs_poly.emplace_back(raw_data_to_polynomial_assume_sorted(this->matlabEngine, poly_factory, raw_rhs));
        }

        // Combine
        std::vector<Polynomial> output_poly;
        output_poly.reserve(input.output_size);
        switch (input.distribution_mode) {
            case PlusParams::DistributionMode::OneToMany:
                for (size_t n = 0; n < input.output_size; ++n) {
                    output_poly.emplace_back(lhs_poly[0] + rhs_poly[n]);
                }
                break;
            case PlusParams::DistributionMode::ManyToOne:
                for (size_t n = 0; n < input.output_size; ++n) {
                    output_poly.emplace_back(lhs_poly[n] + rhs_poly[0]);
                }
                break;
            case PlusParams::DistributionMode::ManyToMany:
                for (size_t n = 0; n < input.output_size; ++n) {
                    output_poly.emplace_back(lhs_poly[n] + rhs_poly[n]);
                }
                break;
        }

        // Attempt to infer if output is a monomial object
        const bool detect_if_monomial = output.size() >= 2;
        bool is_monomial = false;
        if (detect_if_monomial) {
            is_monomial = std::all_of(output_poly.begin(), output_poly.end(), [](const Polynomial& poly) {
                return poly.is_monomial();
            });
        }

        // Export polynomials
        matlab::data::ArrayFactory factory;
        PolynomialExporter exporter{this->matlabEngine, factory,
                                    matrixSystem.Context(), matrixSystem.Symbols(), poly_factory.zero_tolerance};
        switch (input.output_mode) {
            case PlusParams::OutputMode::String: {
                matlab::data::StringArray string_out = factory.createArray<matlab::data::MATLABString>(
                        input.output_shape);

                std::transform(output_poly.cbegin(), output_poly.cend(), string_out.begin(),
                               [&exporter](const Polynomial &poly) -> matlab::data::MATLABString {
                                   return exporter.string(poly);
                               });
                output[0] = std::move(string_out);
            } break;
            case PlusParams::OutputMode::SymbolCell: {
                matlab::data::CellArray cell_out = factory.createCellArray(input.output_shape);
                std::transform(output_poly.cbegin(), output_poly.cend(), cell_out.begin(),
                               [&exporter](const Polynomial &poly) -> matlab::data::CellArray {
                                   return exporter.symbol_cell(poly);
                               });
                output[0] = std::move(cell_out);
            } break;
            case PlusParams::OutputMode::SequencesWithSymbolInfo: {
                if (is_monomial) {
                    assert(detect_if_monomial);
                    auto fms = exporter.monomial_sequence_cell_vector(output_poly, input.output_shape, true);
                    output[0] = fms.move_to_cell(factory);
                } else {
                    output[0] = exporter.sequence_cell_vector(output_poly, true);
                }
            } break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output format for plus.");
        }

        // Write if output object is purely monomial.
        if (detect_if_monomial) {
            assert (output.size() >= 2);
            output[1] = factory.createScalar<bool>(is_monomial);
        }
    }
}