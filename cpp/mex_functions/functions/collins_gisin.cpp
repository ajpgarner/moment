/**
 * collins_gisin_table.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "collins_gisin.h"

#include "scenarios/locality/collins_gisin.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "storage_manager.h"

#include "export/export_collins_gisin.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "utilities/utf_conversion.h"

#include "mex.hpp"


namespace Moment::mex::functions  {

    namespace {
//
//        matlab::data::TypedArray<uint64_t>
//        export_cgt_real_basis(matlab::engine::MATLABEngine &matlabEngine,
//                              const Moment::Locality::CollinsGisin& cgi) {
//
//            matlab::data::ArrayFactory factory;
//            matlab::data::ArrayDimensions dimensions{cgi.Dimensions};
//            matlab::data::TypedArray<uint64_t> output
//                = factory.createArray<uint64_t>(std::move(dimensions));
//
//
//            const auto& readData = cgi.RealIndices();
//            auto writeIter = output.begin();
//            auto readIter = readData.begin();
//            while (writeIter != output.end() && readIter != readData.end()) {
//                *writeIter = *readIter + 1; // +1 for matlab indexing
//                ++writeIter;
//                ++readIter;
//            }
//
//            return output;
//        }

    }

    CollinsGisinParams::CollinsGisinParams(SortedInputs &&inputIn)
            : SortedInputs(std::move(inputIn)) {

        // Get matrix system class
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // See if output type is set
        if (this->flags.contains(u"symbols")) {
            this->outputType = OutputType::SymbolIds;
        } else if (this->flags.contains(u"sequences")) {
            this->outputType = OutputType::Sequences;
        } else if (this->flags.contains(u"strings")) {
            this->outputType = OutputType::SequenceStrings;
        }
    }

    CollinsGisin::CollinsGisin(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {

        this->flag_names.emplace(u"symbols");
        this->flag_names.emplace(u"sequences");
        this->flag_names.emplace(u"strings");

        this->mutex_params.add_mutex({u"symbols", u"sequences", u"strings"});

        this->min_outputs = 1;
        this->max_outputs = 2;

        this->min_inputs = 1;
        this->max_inputs = 1;
    }


    void CollinsGisin::extra_input_checks(CollinsGisinParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }



    void CollinsGisin::operator()(IOArgumentRange output, CollinsGisinParams &input) {

        // Check output
        if (output.size() == 2 && input.outputType == CollinsGisinParams::OutputType::SequenceStrings) {
            throw_error(this->matlabEngine, errors::too_many_outputs,
                        "Two outputs only expected for 'sequences' and 'symbols' output mode.");
        }

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

            CollinsGisinExporter cge{this->matlabEngine, lsm->localityContext, lsm->Symbols()};

            // Export whole matrix?
            if (output.size() >= 1) {
                switch (input.outputType) {
                    case CollinsGisinParams::OutputType::SymbolIds:
                        try {
                            output[0] = cge.symbol_ids(cg);
                            if (output.size() > 1) {
                                output[1] = cge.basis_elems(cg);
                            }
                        } catch (const Moment::Locality::errors::BadCGError& bcge) {
                            throw_error(this->matlabEngine, "missing_cg", bcge.what());
                        }
                        break;
                    case CollinsGisinParams::OutputType::Sequences:
                        output[0] = cge.sequences(cg);
                        if (output.size() > 1) {
                            output[1] = cge.hashes(cg);
                        }
                        break;
                    case CollinsGisinParams::OutputType::SequenceStrings: {
                        auto formatter = this->settings->get_locality_formatter();
                        assert(formatter);
                        output[0] = cge.strings(cg, *formatter);
                    }
                    break;
                    default:
                        throw_error(this->matlabEngine, errors::internal_error, "Unknown output type.");
                }

            }
        } catch (const Moment::errors::missing_component& mce) {
            throw_error(this->matlabEngine, "missing_cg", mce.what());
        }

    }
}