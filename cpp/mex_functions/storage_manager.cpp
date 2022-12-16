/**
 * storage_manager.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "storage_manager.h"

namespace Moment::mex {

    StorageManager& getStorageManager() {
        static StorageManager manager{};
        return manager;
    }
}