/**
 * collins_gisin_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "collins_gisin.h"

#include "scenarios/locality/collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "storage_manager.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "mex.hpp"


namespace Moment::mex::functions  {

    namespace {

        matlab::data::TypedArray<uint64_t>
        export_cgt_real_basis(matlab::engine::MATLABEngine &matlabEngine,
                              const Moment::Locality::CollinsGisin& cgi) {

            matlab::data::ArrayFactory factory;
            matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
            matlab::data::TypedArray<uint64_t> output
                = factory.createArray<uint64_t>(std::move(dimensions));


            const auto& readData = cgi.RealIndices();
            auto writeIter = output.begin();
            auto readIter = readData.begin();
            while (writeIter != output.end() && readIter != readData.end()) {
                *writeIter = *readIter + 1; // +1 for matlab indexing
                ++writeIter;
                ++readIter;
            }

            return output;
        }

        matlab::data::TypedArray<uint64_t>
        export_cgt_symbol_ids(matlab::engine::MATLABEngine &matlabEngine,
                              const Moment::Locality::CollinsGisin& cgi) {


            matlab::data::ArrayFactory factory;
            matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
            matlab::data::TypedArray<uint64_t> output
                = factory.createArray<uint64_t>(std::move(dimensions));

            const auto& readData = cgi.Symbols();
            auto writeIter = output.begin();
            auto readIter = readData.begin();
            while (writeIter != output.end() && readIter != readData.end()) {
                *writeIter = *readIter;
                ++writeIter;
                ++readIter;
            }

            return output;
        }

        matlab::data::TypedArray<matlab::data::MATLABString>
        export_cgt_sequences(matlab::engine::MATLABEngine &matlabEngine,
                              const Moment::Locality::CollinsGisin& cgi) {
            matlab::data::ArrayFactory factory;
            matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
            matlab::data::TypedArray<matlab::data::MATLABString> output
                = factory.createArray<matlab::data::MATLABString>(std::move(dimensions));

            const auto& readData = cgi.Sequences();
            auto writeIter = output.begin();
            auto readIter = readData.begin();
            while (writeIter != output.end() && readIter != readData.end()) {
                std::string inStr{cgi.context.format_sequence(*readIter)};
                *writeIter = matlab::engine::convertUTF8StringToUTF16String(inStr);
                ++writeIter;
                ++readIter;
            }

            return output;
        }


        matlab::data::Array
        export_cg_tensor(matlab::engine::MATLABEngine &matlabEngine,
                        CollinsGisinParams::OutputType method,
                        const Moment::Locality::CollinsGisin& cgi) {

            switch (method) {
                default:
                case CollinsGisinParams::OutputType::RealBasis:
                    return export_cgt_real_basis(matlabEngine, cgi);
                case CollinsGisinParams::OutputType::SymbolIds:
                    return export_cgt_symbol_ids(matlabEngine, cgi);
                case CollinsGisinParams::OutputType::Sequences:
                    return export_cgt_sequences(matlabEngine, cgi);
            }
        }
    }

    CollinsGisinParams::CollinsGisinParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputIn)
            : SortedInputs(std::move(inputIn)) {

        // Get matrix system class
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // See if output type is set
        if (this->flags.contains(u"symbols")) {
            this->outputType = OutputType::SymbolIds;
        } else if (this->flags.contains(u"sequences")) {
            this->outputType = OutputType::Sequences;
        } else {
            this->outputType = OutputType::RealBasis;
        }
    }

    CollinsGisin::CollinsGisin(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"collins_gisin") {

        this->flag_names.emplace(u"basis");
        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");

        this->mutex_params.add_mutex({u"basis", u"symbols", u"sequences"});

        this->min_outputs = 1;
        this->max_outputs = 1;

        this->min_inputs = 1;
        this->max_inputs = 1;
    }


    void CollinsGisin::extra_input_checks(CollinsGisinParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }

    void CollinsGisin::operator()(IOArgumentRange output, CollinsGisinParams &input) {

        // Get stored moment matrix
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent

        // Get read lock
        auto lock = msPtr->get_read_lock();
        const MatrixSystem& system = *msPtr;

        // Create (or retrieve) CG information
        try {
            const auto * lsm = dynamic_cast<const Locality::LocalityMatrixSystem *>(&system);
            if (nullptr == lsm) {
                throw_error(this->matlabEngine, errors::bad_cast, "MatrixSystem was not a LocalityMatrixSystem.");
            }

            const auto &cg = lsm->CollinsGisin();

            // Export whole matrix?
            if (output.size() >= 1) {
                // Export symbols
                output[0] = export_cg_tensor(this->matlabEngine, input.outputType, cg);
            }
        } catch (const Moment::errors::missing_component& mce) {
            throw_error(this->matlabEngine, "missing_cg", mce.what());
        }

    }
}