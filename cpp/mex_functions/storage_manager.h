/**
 * storage_manager.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "utilities/persistent_storage.h"

#include "matrix_system/matrix_system.h"
#include "environmental_variables.h"
#include "logging/logger.h"

namespace Moment::mex {

    namespace errors {
        constexpr char bad_signature[] = "bad_signature";
        constexpr char object_not_found[] = "object_not_found";
    }

    /**
     * Singleton class, for storing persistent objects.
     */
    class StorageManager {
    public:
        static constexpr uint64_t matrix_system_signature = make_signature({'m','s','y','s'});
        static constexpr uint64_t settings_signature      = make_signature({'e','n','v','v'});
        static constexpr uint64_t logger_signature        = make_signature({'l','o','g','r'});

        PersistentStorage<MatrixSystem> MatrixSystems{StorageManager::matrix_system_signature};
        PersistentStorageMonoid<EnvironmentalVariables> Settings{StorageManager::settings_signature};
        PersistentStorageMonoid<class Logger> Logger{StorageManager::logger_signature};

    public:
        StorageManager() = default;
        StorageManager(const StorageManager& rhs) = delete;
        StorageManager(StorageManager&& rhs) = delete;

        /** Empties all storage. */
        void reset_all() noexcept {
            this->MatrixSystems.clear();
            this->Logger.reset();
            this->Settings.reset();
        }
    };

}