/**
 * mex_entry.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Entry point for Moment mex function suite.
 */

#include "mex.hpp"
#include "mexAdapter.hpp"

#include "mex_main.h"

#include "storage_manager.h"
#include "logging/in_memory_logger.h"

#include <stdexcept>

/**
 * MexFunction lifetime is effectively static (from first invocation of mtk, until 'clear mtk').
 */
class MexFunction : public matlab::mex::Function {
    private:
        /** Data that persists between function calls (i.e. 'static data').
         * Nominally thread-safe.
         */
        Moment::mex::StorageManager persistentStorage;

    public:
        /** De facto DLL entry point */
        MexFunction() {

            // Ensure environmental variables are loaded
            persistentStorage.Settings.create_if_empty<Moment::mex::EnvironmentalVariables>();

            // Ensure a logger exists
            if constexpr (Moment::debug_mode) {
                persistentStorage.Logger.create_if_empty<Moment::mex::InMemoryLogger>();
            } else {
                persistentStorage.Logger.create_if_empty<Moment::mex::IgnoreLogger>();
            }

        }

        /** De facto DLL exit point */
        ~MexFunction() noexcept {
            try {
                this->persistentStorage.reset_all();
            } catch(std::exception& e) {
                // If destructor fails, crash.
                std::terminate();
            }
        }

        /** Function call */
        void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) override {
            Moment::mex::MexMain executor{getEngine(), this->persistentStorage};

            executor(Moment::mex::IOArgumentRange(outputs.begin(), outputs.begin() + outputs.size()),
                     Moment::mex::IOArgumentRange(inputs.begin(), inputs.begin() + inputs.size()));
        }
};
