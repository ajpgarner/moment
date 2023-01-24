/**
 * persistent_storage.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

namespace Moment {

    class persistent_object_error : public std::runtime_error {
    public:
        uint64_t key;

        persistent_object_error(uint64_t key, const std::string &what)
                : key{key}, std::runtime_error{what} { }
    };

    class bad_signature_error : public persistent_object_error {
    public:
        bad_signature_error(uint64_t key, uint32_t actual_sig, uint32_t expected_sig)
            : persistent_object_error(key, make_msg(actual_sig, expected_sig)) { }

        static std::string make_msg(uint32_t actual_sig, uint32_t expected_sig);
    };

    class not_found_error : public persistent_object_error {
    public:
        not_found_error(uint64_t key, uint32_t supplied_id)
            : persistent_object_error(key, make_msg(supplied_id)) { }

        static std::string make_msg(uint32_t supplied_id);
    };


    class PersistentStorageBase {
    public:
        const uint32_t signature;

        /**
        * Create an object store, for thread-safe static retrieval (persistent between subsequent calls from matlab).
        * @param signature Prefix for valid object IDs.
        */
        explicit PersistentStorageBase(const uint32_t signature) : signature{signature} { }

        /**
         * Check if an item key has a matching signature with this bank
         * @param itemKey The item key
         * @return True, if the signature matches this bank
         */
        [[nodiscard]] constexpr bool check_signature(uint64_t itemKey) const noexcept {
            return static_cast<uint32_t>((itemKey & 0xFFFFFFFF00000000) >> 32) == signature;
        };
    };

    /**
     * Monoid storage manager
     * @tparam class_t The class to store
     */
    template<class class_t>
    class PersistentStorageMonoid : public PersistentStorageBase {
    private:
        /** The stored object */
        std::shared_ptr<class_t> the_object;

        /** Mutex is for setting storage pointer!! class_t must maintain its own thread safety if not constant. */
        mutable std::shared_mutex theMutex;
    public:

        explicit PersistentStorageMonoid(uint32_t signature, std::shared_ptr<class_t> input_obj)
            : PersistentStorageBase{signature}, the_object{std::move(input_obj)} {
        }

        explicit PersistentStorageMonoid(uint32_t signature, std::unique_ptr<class_t> input_obj)
            : PersistentStorageBase{signature}, the_object{std::move(input_obj)} { }

        explicit PersistentStorageMonoid(uint32_t signature)
                : PersistentStorageBase{signature}, the_object{nullptr} {
        }

        [[nodiscard]] std::shared_ptr<class_t> get() const {
            std::shared_lock lock{theMutex};
            return the_object;
        }

        void set(std::shared_ptr<class_t> input) {
            std::unique_lock lock{theMutex};
            this->the_object.swap(input);
        }

        [[nodiscard]] bool empty() const {
            std::unique_lock lock{theMutex};
            return !static_cast<bool>(this->the_object);
        }

        template<typename... Args>
        std::shared_ptr<class_t> create_if_empty(Args... args) {
            // If object already exists, just return it.
            std::shared_lock read_lock{theMutex};
            if (this->the_object) {
                return this->the_object;
            }
            read_lock.unlock();

            // No object, so we try to create one...
            std::unique_lock write_lock{theMutex};
            // Make sure another thread did not construct object while we were waiting for exclusive lock
            if (this->the_object) {
                return this->the_object;
            }
            // Create object
            this->the_object = std::make_shared<class_t>(std::forward<Args>(args)...);
            return this->the_object;
        }
    };


    /**
     * Storage manager for shared pointers of class. Should be as thread-safe as the classes being stored.
     * @tparam class_t The class to store
     */
    template<class class_t>
    class PersistentStorage : public PersistentStorageBase {
    public:
        using map_type = typename std::map<uint32_t, std::shared_ptr<class_t>>;

    private:
        map_type objects;
        uint32_t nextID = 0;
        mutable std::shared_mutex theMutex;

    public:
        /**
         * Create a bank of objects, for thread-safe static retrieval (persistent between subsequent calls from matlab).
         * @param signature Prefix for valid object IDs.
         */
        explicit PersistentStorage(uint32_t signature) : PersistentStorageBase{signature}, nextID(0) { }

        /**
         * Get the index associated with the supplied key
         * @param itemKey The item key
         * @return The index part of the key
         */
        constexpr static uint32_t get_index(uint64_t itemKey) noexcept {
            return static_cast<uint32_t>(itemKey & 0x00000000FFFFFFFF);
        };

        /**
         * Return pointer to object stored with key.
         * Shared-ownership pointer emitted, to be thread-safe at pointer level with release(itemKey) on another thread.
         * @param itemKey The item key.
         * @return Shared pointer to retrieved object.
         * @throws persistent_object_error If item with supplied key cannot be retrieved.
         */
        std::shared_ptr<class_t> get(uint64_t itemKey) const {
            const std::shared_lock<std::shared_mutex> lock(theMutex);
            auto itemIter = this->findOrThrow(itemKey);
            return itemIter->second;
        }

        /**
         * Remove item with supplied key from bank. Deletion is thread-safe.
         * @param itemKey The item key.
         * @throws persistent_object_error If item with supplied key cannot be found.
         */
        void release(uint64_t itemKey) {
            const std::unique_lock<std::shared_mutex> lock(theMutex);
            auto itemIter = this->findOrThrow(itemKey);
            this->objects.erase(itemIter);
        }

        /**
         * Save item in bank. Insertion is thread-safe.
         * @param obj Owning pointer to item to store. Ownership is transferred to bank.
         * @return Key for retrieving stored item.
         */
        uint64_t store(std::unique_ptr<class_t> obj) {
            const std::unique_lock<std::shared_mutex> lock(theMutex);

            const uint64_t key = (static_cast<uint64_t>(signature) << 32) + static_cast<uint64_t>(nextID);
            this->objects.emplace_hint(this->objects.end(), nextID, std::move(obj));

            ++nextID;
            return key;
        }

        /**
         * Save item in bank. Insertion is thread-safe.
         * @param obj Shared-ownerhsip pointer to item to store. Ownership is shared with bank.
         * @return Key for retrieving stored item.
         */
        uint64_t store(std::shared_ptr<class_t> obj) {
            const std::unique_lock<std::shared_mutex> lock(theMutex);

            const uint64_t key = (static_cast<uint64_t>(signature) << 32) + static_cast<uint64_t>(nextID);
            this->objects.emplace_hint(this->objects.end(), nextID, std::move(obj));

            ++nextID;
            return key;
        }

        /**
         * Get first item in bank. Thread-safe, in that either an item or nullptr is returned.
         * Return, pair: item index and shared pointer (cf. [0xFFFFFFFF, nullptr] - if at end).
         */
         auto first() const {
            const std::unique_lock<std::shared_mutex> lock(theMutex);
            if (this->objects.empty()) {
                return std::make_pair(uint32_t{0xFFFFFFFF}, std::shared_ptr<class_t>(nullptr));
            }
            auto first_iter = this->objects.begin();
            return std::make_pair(first_iter->first, first_iter->second);
         }

        /**
         * Get next item in bank. Thread-safe, in that either an item or nullptr is returned.
         * Return, pair: item index and shared pointer (cf. [0xFFFFFFFF, nullptr] - if at end).
         */
         auto next(uint32_t previous_id) const {
            const std::unique_lock<std::shared_mutex> lock(theMutex);
            auto the_next = this->objects.upper_bound(previous_id);
            if (the_next == this->objects.cend()) {
                return std::make_pair(uint32_t{0xFFFFFFFF}, std::shared_ptr<class_t>(nullptr));
            }
            return std::make_pair(the_next->first, the_next->second);
         }

        /**
         * Return total number of items in the bank. Thread-safe.
         * @return Number of items in bank.
         */
        inline size_t size() const noexcept {
            const std::shared_lock<std::shared_mutex> lock(theMutex);
            return this->objects.size();
        }

        /**
         * Test if the bank is empty. Thread-safe.
         * @return Number of items in bank.
         */
        inline bool empty() const noexcept {
            const std::shared_lock<std::shared_mutex> lock(theMutex);
            return this->objects.empty();
        }


    private:
        auto findOrThrow(uint64_t itemKey) const {
            // mutex should be held in thread before this is called...

            // Match signature, or throw
            if (!this->check_signature(itemKey)) {
                throw bad_signature_error(itemKey, (itemKey >> 32), signature);
            }

            // Find item, or throw
            const auto item_id = PersistentStorage<class_t>::get_index(itemKey);
            auto itemIter = this->objects.find(item_id);
            if (itemIter == this->objects.end()) {
                throw not_found_error(itemKey, item_id);
            }

            return itemIter;
        }
    };


    /**
     * Convert 4-byte sequence to 32-bit integer.
     */
    static constexpr uint32_t make_signature(const std::array<char, 4>& input) noexcept {
        return static_cast<uint32_t>(input[0])
            + (static_cast<uint32_t>(input[1]) << 8)
            + (static_cast<uint32_t>(input[2]) << 16)
            + (static_cast<uint32_t>(input[3]) << 24);
    }


}