/**
 * add_symmetry.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "new_symmetrized_matrix_system.h"

#include "storage_manager.h"

#include "scenarios/derived/lu_map_core_processor.h"
#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "eigen/export_eigen_sparse.h"
#include "eigen/read_eigen_sparse.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    NewSymmetrizedMatrixSystemParams::NewSymmetrizedMatrixSystemParams(SortedInputs&& raw_inputs)
        : SortedInputs(std::move(raw_inputs)) {
        // Get matrix system ID
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // Read generators
        if (this->inputs[1].getType() != matlab::data::ArrayType::CELL) {
            throw_error(matlabEngine, errors::bad_param, "Second argument must be a cell array of group generators.");
        }

        // Following link promises this is a reference not a copy. I don't know if I believe it.
        // https://www.mathworks.com/help/matlab/matlab_external/avoid-copies-of-large-arrays.html
        const matlab::data::TypedArray<matlab::data::Array> as_cell_array = this->inputs[1];
        size_t expected_dimension = 0;
        size_t cell_index = 0;
        for (const auto& elem : as_cell_array) {
            switch(elem.getType()) {
                case matlab::data::ArrayType::SINGLE:
                case matlab::data::ArrayType::DOUBLE:
                case matlab::data::ArrayType::INT8:
                case matlab::data::ArrayType::INT16:
                case matlab::data::ArrayType::INT32:
                case matlab::data::ArrayType::INT64:
                case matlab::data::ArrayType::UINT8:
                case matlab::data::ArrayType::UINT16:
                case matlab::data::ArrayType::UINT32:
                case matlab::data::ArrayType::UINT64:
                case matlab::data::ArrayType::MATLAB_STRING:
                    break;
                default:{
                    std::stringstream errSS;
                    errSS << "Error reading element " << (cell_index+1) << ": element could not be parsed as a real matrix.";
                    throw_error(matlabEngine, errors::bad_param, errSS.str());
                }
            }

            // Check element is a square matrix
            auto dims = elem.getDimensions();
            if ((dims.size() != 2) || (dims[0] != dims[1])) {
                std::stringstream errSS;
                errSS << "Error reading element " << (cell_index+1) << ": element was not a square matrix.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }

            // Check dimensions
            if (cell_index == 0) {
                expected_dimension = dims[0];
            } else if (dims[0] != expected_dimension) {
                std::stringstream errSS;
                errSS << "Error reading element " << (cell_index+1) << ": expected a "
                      << expected_dimension << " x " << expected_dimension << " matrix, to match first generator dimensions.";
                throw_error(matlabEngine, errors::bad_param, errSS.str());
            }

            ++cell_index;
        }

        // Read maximum element size, if one is set
        if (this->inputs.size() >= 3) {
            if (!castable_to_scalar_int(this->inputs[2])) {
                throw_error(matlabEngine, errors::bad_param,
                            "Maximum word length, if provided, must be a scalar non-negative integer.");
            }
            this->max_word_length = read_as_uint64(this->matlabEngine, this->inputs[2]);
        }

        // Is a subgroup limit specified?
        auto maxSGiter = this->params.find(u"max_subgroup");
        if (maxSGiter != this->params.end()) {
            this->max_subgroup = read_positive_integer<size_t>(matlabEngine, "Parameter 'max_subgroup'",
                                                               maxSGiter->second, 0);
        } else {
            this->max_subgroup = 0;
        }

    }

    NewSymmetrizedMatrixSystem::NewSymmetrizedMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
         : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->param_names.emplace(u"max_subgroup");
    }

    void NewSymmetrizedMatrixSystem::operator()(IOArgumentRange output, NewSymmetrizedMatrixSystemParams &input) {

        using namespace Moment::Symmetrized;

        // Get matrix system:
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent
        auto& matrixSystem = *msPtr;

        // Convert input to eigen sparse matrices
        const auto generators = read_eigen_sparse_array(matlabEngine, input.inputs[1]);

        // Output read matrices
        if (this->debug) {
            std::stringstream ss;
            ss << "Parsed " << generators.size() << " generators:\n";
            size_t gIndex = 0;
            for (const auto& gen : generators) {
                ss << "Generator #" << (gIndex+1) << ":\n" << gen << "\n";
                ++gIndex;
            }
            print_to_console(matlabEngine, ss.str());
        }

        std::vector<repmat_t> group_elements = (input.max_subgroup > 0)
                                             ? Group::dimino_generation(generators, input.max_subgroup)
                                             : Group::dimino_generation(generators);

        // Export expanded matrices, if requested
        if (output.size() >= 2) {
            matlab::data::ArrayFactory factory;
            output[1] = export_eigen_sparse_array(matlabEngine, factory, group_elements);
        }

        std::unique_ptr<Representation> rep = std::make_unique<Representation>(1, std::move(group_elements));
        std::unique_ptr<Group> groupPtr;
        try {
            groupPtr = std::make_unique<Group>(matrixSystem.Context(), std::move(rep));
        } catch (std::runtime_error& rte) {
            std::stringstream errSS;
            errSS << "Error creating symmetry group: " << rte.what();
            throw_error(matlabEngine, errors::bad_param, errSS.str());
        }

        // Check input matrix system has required symbols. [Write-locks source matrix system].
        auto input_system_lock = matrixSystem.get_read_lock();
        if (input.max_word_length > 0) {
            input_system_lock.unlock(); // unlock as gen dict will try to acquire a write lock.
            matrixSystem.generate_dictionary(input.max_word_length);
            input_system_lock.lock();
        } else {
            auto hmm = matrixSystem.highest_moment_matrix();
            if (hmm > 0) {
                input.max_word_length = 2 * hmm;
            }
        }
        if (0 == input.max_word_length) {
            std::stringstream errSS;
            errSS << "Maximum operator word length for map could not be automatically deduced.\n"
                 << "Either first create a moment matrix of the desired maximum size in the base system, "
                 << "or manually supply the size of the longest operator string to be mapped.";
            throw_error(matlabEngine, errors::bad_param, errSS.str());
        }

        // Now, create new matrix system with group
        std::unique_ptr<MatrixSystem> smsPtr =
                std::make_unique<SymmetrizedMatrixSystem>(std::move(msPtr), std::move(groupPtr),
                                                          input.max_word_length,
                                                          std::make_unique<Derived::LUMapCoreProcessor>());
        // Print map information
        if (verbose) {
            print_to_console(this->matlabEngine,
                             dynamic_cast<SymmetrizedMatrixSystem&>(*smsPtr).describe_map());
        }

        // Store matrix system (makes visible to other threads!)
        auto nms_id = this->storageManager.MatrixSystems.store(std::move(smsPtr));

        // Write output ID of symmetrized system
        if (output.size() >= 1) {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar<uint64_t>(nms_id);
        }
    }

    void NewSymmetrizedMatrixSystem::extra_input_checks(NewSymmetrizedMatrixSystemParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }

        ParameterizedMexFunction::extra_input_checks(input);
    }
}