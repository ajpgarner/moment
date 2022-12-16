/**
 * probability_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "probability_table.h"

#include "storage_manager.h"

#include "operators/matrix/operator_matrix.h"
#include "operators/matrix/moment_matrix.h"
#include "operators/inflation/inflation_matrix_system.h"
#include "operators/locality/locality_implicit_symbols.h"
#include "operators/locality/locality_matrix_system.h"
#include "fragments/export_implicit_symbols.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"


namespace Moment::mex::functions {

    namespace {
        class IndexReaderVisitor {
        private:
            matlab::engine::MATLABEngine &engine;
        public:
            using return_type = std::vector<ProbabilityTableParams::RawTriplet>;

            explicit IndexReaderVisitor(matlab::engine::MATLABEngine &matlabEngine) : engine{matlabEngine} { }

            /** Dense input -> dense monolithic output */
            template<std::convertible_to<size_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
                std::vector<ProbabilityTableParams::RawTriplet> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                assert((dims[1] == 2) || (dims[1] == 3));
                const bool triplet = (dims[1] == 3);

                for (size_t row = 0; row < dims[0]; ++row) {
                    if (matrix[row][0] < 1) {
                        throw errors::BadInput{errors::bad_param, "Party index should be positive integer."};
                    }
                    if (matrix[row][1] < 1) {
                        throw errors::BadInput{errors::bad_param, "Measurement index should be positive integer."};
                    }
                    if (triplet && (matrix[row][2] < 1)) {
                        throw errors::BadInput{errors::bad_param, "Outcome index should be positive integer."};
                    }
                    if (triplet) {
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(matrix[row][0] - 1),
                                            static_cast<size_t>(matrix[row][1] - 1),
                                            static_cast<size_t>(matrix[row][2] - 1));
                    } else {
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(matrix[row][0] - 1),
                                            static_cast<size_t>(matrix[row][1] - 1), 0);
                    }
                }
                return output;
            }

            /** Dense input -> dense monolithic output */
            return_type string(const matlab::data::StringArray& matrix) {
                std::vector<ProbabilityTableParams::RawTriplet> output;
                const auto dims = matrix.getDimensions();
                assert(dims.size() == 2);
                const bool triplet = (dims[1] == 3);

                for (size_t row = 0; row < dims[0]; ++row) {
                    auto party_raw = SortedInputs::read_positive_integer(engine, "Party index",
                                                                         matrix[row][0], 1);
                    auto mmt_raw = SortedInputs::read_positive_integer(engine, "Measurement index",
                                                                         matrix[row][1], 1);

                    if (!triplet) {
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(party_raw - 1),
                                            static_cast<size_t>(mmt_raw - 1), 0);
                    } else {
                        auto outcome_raw = SortedInputs::read_positive_integer(engine, "Outcome index",
                                                                               matrix[row][2], 1);
                        // from matlab index to C++ index
                        output.emplace_back(static_cast<size_t>(party_raw - 1),
                                            static_cast<size_t>(mmt_raw - 1),
                                            static_cast<size_t>(outcome_raw - 1));
                    }
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<IndexReaderVisitor>);
        static_assert(concepts::VisitorHasString<IndexReaderVisitor>);

    }

    ProbabilityTableParams::ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputIn)
            : SortedInputs(std::move(inputIn)) {
        // Get matrix system ID
        this->matrix_system_key = read_positive_integer(matlabEngine, "Reference id", this->inputs[0], 0);

        // For single input, just get whole table
        if (this->inputs.size() < 2) {
            this->export_mode = ExportMode::WholeTable;
            return;
        }

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

        // Check input dimensions
        const auto keyDims = this->inputs[1].getDimensions();
        if ((2 != keyDims.size()) || ((keyDims[1] != 3) && (keyDims[1] != 2))) {
            throw errors::BadInput{errors::bad_param,
                                   std::string("Measurement indices should be written as a")
                                    + " Nx3 matrix (e.g., [[party, mmt, outcome]; [party mmt, outcome]]),"
                                    + " or as a Nx2 matrix (e.g., [[party, mmt]; [party, mmt]])."};

        }
        this->export_mode = (keyDims[1] == 3) ? ExportMode::OneOutcome : ExportMode::OneMeasurement;

        this->requested_indices = DispatchVisitor(matlabEngine, this->inputs[1], IndexReaderVisitor{matlabEngine});
    }

    std::vector<PMIndex> ProbabilityTableParams::requested_measurement() const {
        std::vector<PMIndex> output{};
        output.reserve(this->requested_indices.size());
        for (const auto& i : this->requested_indices) {
            output.emplace_back(i.first, i.second);
        }

        // Check for duplicate parties
        std::set<size_t> partySet;
        for (const auto &pm: output) {
            partySet.emplace(pm.party);
        }
        if (partySet.size() != output.size()) {
            throw errors::BadInput{errors::bad_param, "No duplicate parties may be specified."};
        }

        // Sort requested indices
        std::sort(output.begin(), output.end(),
                  [](const auto &lhs, const auto &rhs) { return lhs.party < rhs.party; });

        return output;
    }

    std::vector<PMOIndex> ProbabilityTableParams::requested_outcome() const {
        std::vector<PMOIndex> output{};
        output.reserve(this->requested_indices.size());
        for (const auto& i : this->requested_indices) {
            output.emplace_back(i.first, i.second, i.third);
        }

        // Check for duplicate parties
        std::set<size_t> partySet;
        for (const auto &pmo: output) {
            partySet.emplace(pmo.party);
        }
        if (partySet.size() != output.size()) {
            throw errors::BadInput{errors::bad_param, "No duplicate parties may be specified."};
        }

        // Sort requested indices
        std::sort(output.begin(), output.end(),
                  [](const auto &lhs, const auto &rhs) { return lhs.party < rhs.party; });

        return output;
    }

    std::vector<OVIndex> ProbabilityTableParams::requested_observables() const {
        std::vector<OVIndex> output{};
        output.reserve(this->requested_indices.size());
        for (const auto& i : this->requested_indices) {
            output.emplace_back(i.first, i.second);
        }
        // Sort requested indices
        std::sort(output.begin(), output.end());

        return output;
    }

    std::vector<OVOIndex> ProbabilityTableParams::requested_ovo() const {
        std::vector<OVOIndex> output{};
        output.reserve(this->requested_indices.size());
        for (const auto& i : this->requested_indices) {
            output.emplace_back(i.first, i.second, i.third);
        }
        // Sort requested indices
        std::sort(output.begin(), output.end());
        return output;
    }

    ProbabilityTable::ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::ProbabilityTable, u"probability_table") {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 2;

        this->flag_names.emplace(u"inflation");
    }

    void ProbabilityTable::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        // Get input
        assert(inputPtr);
        auto& input = dynamic_cast<ProbabilityTableParams&>(*inputPtr);

        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent

        // Get read lock
        auto lock = msPtr->get_read_lock();
        const MatrixSystem& system = *msPtr;

        // Attempt to read as locality system
        const auto * lms = dynamic_cast<const LocalityMatrixSystem *>(&system);
        if (nullptr != lms) {
            export_locality(output, input, *lms);
            return;
        }

        // Attempt to read as inflation system
        const auto * ims = dynamic_cast<const InflationMatrixSystem *>(&system);
        if (nullptr != ims) {
            export_inflation(output, input, *ims);
            return;
        }

        // Could not read...!
        throw_error(this->matlabEngine, errors::bad_cast,
                    "MatrixSystem was neither a LocalityMatrixSystem, or an InflationMatrixSystem.");

    }

    std::unique_ptr<SortedInputs> ProbabilityTable::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto tx = std::make_unique<ProbabilityTableParams>(this->matlabEngine, std::move(*input));
        if (!this->storageManager.MatrixSystems.check_signature(tx->matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
        return tx;
    }

    void ProbabilityTable::export_locality(IOArgumentRange output,
                                          ProbabilityTableParams& input, const LocalityMatrixSystem& lms) {

        const LocalityContext& context = lms.localityContext;

        // Create (or retrieve) implied sequence object
        const LocalityImplicitSymbols& implSym = lms.ImplicitSymbolTable();

        // Export whole table?
        if (input.export_mode == ProbabilityTableParams::ExportMode::WholeTable) {
            // Export symbols
            output[0] = export_implied_symbols(this->matlabEngine, implSym);
            return;
        }

        if (input.export_mode == ProbabilityTableParams::ExportMode::OneMeasurement) {
            auto requested_measurement = input.requested_measurement();
            // Check inputs are okay:
            if (requested_measurement.size() > lms.MaxRealSequenceLength()) {
                throw_error(this->matlabEngine, errors::bad_param,
                    "A moment matrix of high enough order to define the requested probability was not specified.");
            }
            for (const auto& pm : requested_measurement) {
                if (pm.party >= context.Parties.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Party index out of range.");
                }
                const auto &party = context.Parties[pm.party];
                if (pm.mmt >= party.Measurements.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Measurement index out of range.");
                }
            }

            // Assign global indices to input.requested_measurement object...
            context.get_global_mmt_index(requested_measurement);

            // Request
            output[0] = export_implied_symbols(this->matlabEngine, implSym, requested_measurement);
            return;
        }


        if (input.export_mode == ProbabilityTableParams::ExportMode::OneOutcome) {
            auto requested_outcome = input.requested_outcome();
            // Check inputs are okay:
            if (requested_outcome.size() > lms.MaxRealSequenceLength()) {
                throw_error(this->matlabEngine, errors::bad_param,
                    "A moment matrix of high enough order to define the requested probability was not specified.");
            }
            for (const auto& pm : requested_outcome) {
                if (pm.party >= context.Parties.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Party index out of range.");
                }
                const auto &party = context.Parties[pm.party];
                if (pm.mmt >= party.Measurements.size()) {
                    throw_error(this->matlabEngine, errors::bad_param, "Measurement index out of range.");
                }
                const auto &mmt = party.Measurements[pm.mmt];
                if (pm.outcome >= mmt.num_outcomes) {
                    throw_error(this->matlabEngine, errors::bad_param, "Outcome index out of range.");
                }
            }
            // Request
            output[0] = export_implied_symbols(this->matlabEngine, implSym, requested_outcome);
            return;
        }



        throw_error(this->matlabEngine, errors::internal_error, "Unknown export type.");
    }

    void ProbabilityTable::export_inflation(IOArgumentRange output,
                                            ProbabilityTableParams& input, const InflationMatrixSystem& ims) {

        const InflationContext& context = ims.InflationContext();

        // Create (or retrieve) implied sequence object
        const InflationImplicitSymbols& implSym = ims.ImplicitSymbolTable();

        // Export whole table?
        if (input.export_mode == ProbabilityTableParams::ExportMode::WholeTable) {
            // Export symbols
            output[0] = export_implied_symbols(this->matlabEngine, implSym);
            return;
        }

        if (input.export_mode == ProbabilityTableParams::ExportMode::OneMeasurement) {
            auto requested_observable = input.requested_observables();
            output[0] = export_implied_symbols(this->matlabEngine, implSym, requested_observable);
            return;
        }

        if (input.export_mode == ProbabilityTableParams::ExportMode::OneOutcome) {
            auto requested_outcome = input.requested_ovo();
            output[0] = export_implied_symbols(this->matlabEngine, implSym, requested_outcome);
            return;
        }

        throw_error(this->matlabEngine, errors::internal_error, "Unknown export type.");
    }


}