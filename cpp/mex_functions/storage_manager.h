/**
 * storage_manager.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "utilities/persistent_storage.h"

//#include "operators/context.h"
#include "operators/moment_matrix.h"

#include <string>
#include <optional>

namespace NPATK::mex {

    namespace errors {
        constexpr char bad_signature[] = "bad_signature";
        constexpr char object_not_found[] = "object_not_found";
    }

    /**
     * Singleton class, for storing persistent objects.
     */
    class StorageManager {
    public:
        PersistentStorage<MomentMatrix> MomentMatrices{make_signature({'m','m','t','m'})};

        friend StorageManager& getStorageManager();

    private:
        StorageManager() = default;

    public:
        StorageManager(const StorageManager& rhs) = delete;
        StorageManager(StorageManager&& rhs) = delete;
    };

    /**
     * Return storage manager singleton.
     */
    [[nodiscard]] StorageManager& getStorageManager();

}