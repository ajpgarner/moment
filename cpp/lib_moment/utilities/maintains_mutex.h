/**
 * maintains_mutex.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <shared_mutex>

namespace Moment {
    class MaintainsMutex {
    public:
        using ReadLock = std::shared_lock<std::shared_mutex>;
        using WriteLock = std::unique_lock<std::shared_mutex>;

    private:
        /** Read-write mutex. */
        mutable std::shared_mutex rwMutex;

    protected:
    public:
        /**
         * Gets a read (shared) lock for accessing data within the object.
         */
        [[nodiscard]] inline ReadLock get_read_lock() const {
            return ReadLock{this->rwMutex};
        }

        /**
         * Gets a write (exclusive) lock for manipulating data within the object.
         */
        [[nodiscard]] inline WriteLock get_write_lock() {
            return WriteLock{this->rwMutex};
        }

        /**
         * True if input is a valid, locked, read lock for this object.
         */
        [[nodiscard]] inline bool is_locked_read_lock(const ReadLock& lock) const {
            return (lock.mutex() == &this->rwMutex) && lock.owns_lock();
        }
        void is_locked_read_lock(ReadLock&& lock) const = delete;

        /**
         * True if input is a valid, locked, write lock for this object.
         */
        [[nodiscard]] inline bool is_locked_write_lock(const WriteLock& lock) const {
            return (lock.mutex() == &this->rwMutex) && lock.owns_lock();
        }
        void is_locked_write_lock(WriteLock&& lock) const = delete;


    };

}