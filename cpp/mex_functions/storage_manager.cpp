/**
 * storage_manager.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "storage_manager.h"

namespace Moment::mex {

    StorageManager& getStorageManager() {
        static StorageManager manager{};
        return manager;
    }
}