/**
 * mtk_function.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "mtk_function.h"
#include "storage_manager.h"
#include "environmental_variables.h"

namespace Moment::mex::functions {
    MTKFunction::MTKFunction(matlab::engine::MATLABEngine& engine, StorageManager& storage, MTKEntryPointID id)
        : matlabEngine(engine), storageManager{storage}, function_id{id} {

        this->settings = std::const_pointer_cast<const EnvironmentalVariables>(storage.Settings.get());
    }

    MTKFunction::~MTKFunction() = default;

    std::unique_ptr<SortedInputs> MTKFunction::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return std::move(input);
    }

    void MTKFunction::setQuiet(bool val) noexcept {
        // Quiet mode only turns on if debug mode not set.
        this->quiet = val && !this->debug;
        if (val) {
            // Turning on quiet mode turns off verbose mode
            this->verbose = false;
        }
    }

    void MTKFunction::setVerbose(bool val) noexcept {
        this->verbose = val;
        if (val) {
            // Turning on verbosity turns off quiet mode
            this->quiet = false;
        } else {
            // Turning off verbosity also turns off debug mode
            this->debug = false;
        }
    }

    void MTKFunction::setDebug(bool val) noexcept {
        this->debug = val;
        if (val) {
            // Turning on debug mode turns on verbosity, and turns off quiet mode
            this->verbose = true;
            this->quiet = false;
        }
    }


}