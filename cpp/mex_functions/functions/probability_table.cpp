/**
 * probability_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "probability_table.h"

#include "storage_manager.h"

#include "matlab_classes/moment_matrix.h"
#include "operators/implicit_symbols.h"
#include "fragments/export_implicit_symbols.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"

namespace NPATK::mex::functions {

    namespace {
        class PMOIndexReaderVisitor {

        private:
            matlab::engine::MATLABEngine &engine;
        public:
            using return_type = std::vector<PMOIndex>;

            explicit PMOIndexReaderVisitor(matlab::engine::MATLABEngine &matlabEngine)
                : engine{matlabEngine} { }

            /** Dense input -> dense monolithic output */
            template<std::convertible_to<size_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                std::vector<PMOIndex> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert(dims[1] == 3);

                for (size_t row = 0; row < dims[0]; ++row) {
                    if (matrix[row][0] < 1) {
                        throw errors::BadInput{errors::bad_param, "Party index should be positive integer."};
                    }
                    if (matrix[row][1] < 1) {
                        throw errors::BadInput{errors::bad_param, "Measurement index should be positive integer."};
                    }
                    if (matrix[row][2] < 1) {
                        throw errors::BadInput{errors::bad_param, "Outcome index should be positive integer."};
                    }
                    output.emplace_back(static_cast<party_name_t>(matrix[row][0]-1),// from matlab index to C++ index
                                        static_cast<mmt_name_t>(matrix[row][1]-1),  // from matlab index to C++ index
                                        static_cast<uint32_t>(matrix[row][2]-1));   // from matlab index to C++ index
                }
                return output;
            }

            /** Dense input -> dense monolithic output */
            return_type string(const matlab::data::StringArray& matrix) {
                std::vector<PMOIndex> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert(dims[1] == 3);
                for (size_t row = 0; row < dims[0]; ++row) {
                    auto party_raw = SortedInputs::read_positive_integer(engine, "Party index",
                                                                         matrix[row][0], 1);
                    auto mmt_raw = SortedInputs::read_positive_integer(engine, "Measurement index",
                                                                         matrix[row][1], 1);
                    auto outcome_raw = SortedInputs::read_positive_integer(engine, "Outcome index",
                                                                         matrix[row][2], 1);

                    output.emplace_back(static_cast<party_name_t>(party_raw - 1), // from matlab index to C++ index
                                        static_cast<mmt_name_t>(mmt_raw - 1),     // from matlab index to C++ index
                                        static_cast<uint32_t>(outcome_raw - 1));  // from matlab index to C++ index
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<PMOIndexReaderVisitor>);
        static_assert(concepts::VisitorHasString<PMOIndexReaderVisitor>);

        class PMIndexReaderVisitor {

        private:
            matlab::engine::MATLABEngine &engine;
        public:
            using return_type = std::vector<PMIndex>;

            explicit PMIndexReaderVisitor(matlab::engine::MATLABEngine &matlabEngine)
                : engine{matlabEngine} { }

            /** Dense input -> dense monolithic output */
            template<std::convertible_to<size_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                std::vector<PMIndex> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert(dims[1] == 2);

                for (size_t row = 0; row < dims[0]; ++row) {
                    if (matrix[row][0] < 1) {
                        throw errors::BadInput{errors::bad_param, "Party index should be positive integer."};
                    }
                    if (matrix[row][1] < 1) {
                        throw errors::BadInput{errors::bad_param, "Measurement index should be positive integer."};
                    }
                    output.emplace_back(static_cast<party_name_t>(matrix[row][0]-1),// from matlab index to C++ index
                                        static_cast<mmt_name_t>(matrix[row][1]-1));   // from matlab index to C++ index
                }
                return output;
            }

            /** Dense input -> dense monolithic output */
            return_type string(const matlab::data::StringArray& matrix) {
                std::vector<PMIndex> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert(dims[1] == 2);
                for (size_t row = 0; row < dims[0]; ++row) {
                    auto party_raw = SortedInputs::read_positive_integer(engine, "Party index",
                                                                         matrix[row][0], 1);
                    auto mmt_raw = SortedInputs::read_positive_integer(engine, "Measurement index",
                                                                         matrix[row][1], 1);

                    output.emplace_back(static_cast<party_name_t>(party_raw - 1), // from matlab index to C++ index
                                        static_cast<mmt_name_t>(mmt_raw - 1));     // from matlab index to C++ index
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<PMIndexReaderVisitor>);
        static_assert(concepts::VisitorHasString<PMIndexReaderVisitor>);

    }

    ProbabilityTableParams::ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&input)
            : SortedInputs(std::move(input)) {

        // Get moment matrix class
        auto [mmClassPtr, fail] = read_as_moment_matrix(matlabEngine, inputs[0]); // Implicit copy...
        if (!mmClassPtr) {
            throw errors::BadInput{errors::bad_param, fail.value()};
        }
        this->moment_matrix_key = mmClassPtr->Key();

        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_mode = ExportMode::WholeTable;
            return;
        }
        // Otherwise, parse as potential PMO index
        const auto keyDims = this->inputs[1].getDimensions();
        if ((2 != keyDims.size()) || ((keyDims[1] != 3) && (keyDims[1] != 2))) {
            throw errors::BadInput{errors::bad_param, std::string("Measurement indices should be written as a")
                                    + " Nx3 matrix (e.g., [[party, mmt, outcome]; [party mmt, outcome]]),"
                                    + " or as a Nx2 matrix (e.g., [[party, mmt]; [party, mmt]])."};
        }

        this->export_mode = (keyDims[1] == 3) ? ExportMode::OneOutcome : ExportMode::OneMeasurement;

        // Check input type
        switch (this->inputs[1].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Index type must be real numeric, or of numeric strings."};
        }

        // Read indices
        if (this->export_mode == ExportMode::OneOutcome) {
            this->requested_outcome = DispatchVisitor(matlabEngine, this->inputs[1],
                                                      PMOIndexReaderVisitor{matlabEngine});

            // Check for duplicate parties
            std::set<size_t> partySet;
            for (const auto &pmo: this->requested_outcome) {
                partySet.emplace(pmo.party);
            }
            if (partySet.size() != this->requested_outcome.size()) {
                throw errors::BadInput{errors::bad_param, "No duplicate parties may be specified."};
            }

            // Sort requested indices
            std::sort(this->requested_outcome.begin(), this->requested_outcome.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs.party < rhs.party; });
        } else {
            this->requested_measurement = DispatchVisitor(matlabEngine, this->inputs[1],
                                                          PMIndexReaderVisitor{matlabEngine});

            // Check for duplicate parties
            std::set<size_t> partySet;
            for (const auto &pm: this->requested_measurement) {
                partySet.emplace(pm.party);
            }
            if (partySet.size() != this->requested_measurement.size()) {
                throw errors::BadInput{errors::bad_param, "No duplicate parties may be specified."};
            }

            // Sort requested indices
            std::sort(this->requested_measurement.begin(), this->requested_measurement.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs.party < rhs.party; });
        }

    }

    ProbabilityTable::ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::ProbabilityTable, u"probability_table") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 2;
    }

    void ProbabilityTable::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        // Get input
        assert(inputPtr);
        auto& input = dynamic_cast<ProbabilityTableParams&>(*inputPtr);

        // Get stored moment matrix
        auto mmPtr = this->storageManager.MomentMatrices.get(input.moment_matrix_key);
        assert(mmPtr);
        const auto& momentMatrix = *mmPtr;
        const auto& context = momentMatrix.context;

        // Create (or retrieve) implied sequence object
        const ImplicitSymbols& implSym = momentMatrix.ImplicitSymbolTable();

        // Export whole table?
        if (input.export_mode == ProbabilityTableParams::ExportMode::WholeTable) {
            // Export symbols
            output[0] = export_implied_symbols(this->matlabEngine, implSym);
            return;
        }

        if (input.export_mode == ProbabilityTableParams::ExportMode::OneMeasurement) {
            // Check inputs are okay:
            if (input.requested_measurement.size() > momentMatrix.max_probability_length) {
                throw_error(this->matlabEngine, errors::bad_param,
                            "Moment matrix is of too low order to define the requested probability.");
            }
            for (const auto& pm : input.requested_measurement) {
                if (pm.party >= context.Parties.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Party index out of range.");
                }
                const auto &party = context.Parties[pm.party];
                if (pm.mmt >= party.Measurements.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Measurement index out of range.");
                }
            }

            // Assign global indices
            context.get_global_mmt_index(input.requested_measurement);

            // Request
            output[0] = export_implied_symbols(this->matlabEngine, implSym, input.requested_measurement);
            return;
        }

        // Otherwise, export one measurement
        // Further sanitize input
        if (input.requested_outcome.size() > momentMatrix.max_probability_length) {
            throw_error(this->matlabEngine, errors::bad_param,
                        "Moment matrix is of too low order to define the requested probability.");
        }
        for (const auto& pmo : input.requested_outcome) {
            if (pmo.party >= context.Parties.size()) {
                throw_error(this->matlabEngine, errors::bad_param, "Party index out of range.");
            }
            const auto& party = context.Parties[pmo.party];
            if (pmo.mmt >= party.Measurements.size()) {
                throw_error(this->matlabEngine, errors::bad_param, "Measurement index out of range.");
            }
            const auto& mmt = party.Measurements[pmo.mmt];
            if (pmo.outcome >= mmt.num_outcomes) {
                throw_error(this->matlabEngine, errors::bad_param, "Outcome index out of range.");
            }
        }

        // Request output
        auto foundRow = implSym.get(input.requested_outcome);
        output[0] = export_implied_symbol_row(this->matlabEngine, momentMatrix, input.requested_outcome, foundRow);
    }

    std::unique_ptr<SortedInputs> ProbabilityTable::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto tx = std::make_unique<ProbabilityTableParams>(this->matlabEngine, std::move(*input));
        if (!this->storageManager.MomentMatrices.check_signature(tx->moment_matrix_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
        return tx;
    }

}