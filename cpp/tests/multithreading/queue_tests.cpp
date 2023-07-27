/**
 * queue_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "multithreading/queue.h"

#include <future>
#include <string>

namespace Moment::Tests {
    using namespace Moment::Multithreading;

    TEST(Multithreading_Queue, Construct_Empty) {
        Queue<std::string> test_queue{};
    }

    TEST(Multithreading_Queue, PushAndPop) {
        Queue<std::string> test_queue{};

        ASSERT_FALSE(test_queue.try_pop_front());

        test_queue.push_back("Hello");
        test_queue.push_back("World");

        auto hello = test_queue.try_pop_front();
        ASSERT_TRUE(hello);
        EXPECT_EQ((**hello), "Hello");

        test_queue.push_back("Cheesecake");

        auto world = test_queue.try_pop_front();
        ASSERT_TRUE(world);
        EXPECT_EQ((**world), "World");

        auto cheesecake = test_queue.try_pop_front();
        ASSERT_TRUE(cheesecake);
        EXPECT_EQ((**cheesecake), "Cheesecake");

        ASSERT_FALSE(test_queue.try_pop_front());
    }

    TEST(Multithreading_Queue, PushAndPopWithWait) {
        Queue<std::string> test_queue{};

        ASSERT_FALSE(test_queue.try_pop_front());

        test_queue.push_back("Hello");
        test_queue.push_back("World");

        auto hello = test_queue.wait_pop_front();
        ASSERT_TRUE(hello);
        EXPECT_EQ((**hello), "Hello");

        test_queue.push_back("Cheesecake");

        auto world = test_queue.wait_pop_front();
        ASSERT_TRUE(world);
        EXPECT_EQ((**world), "World");

        auto cheesecake = test_queue.wait_pop_front();
        ASSERT_TRUE(cheesecake);
        EXPECT_EQ((**cheesecake), "Cheesecake");

        ASSERT_FALSE(test_queue.try_pop_front());
    }

    TEST(Multithreading_Queue, WaitInParallel) {
        Queue<std::string> test_queue;

        std::promise<std::string> hello_promise;
        std::future<std::string> hello_future = hello_promise.get_future();

        std::thread listener{[&]() {
            auto node = test_queue.wait_pop_front();
            hello_promise.set_value(**node);
        }};

        test_queue.emplace_back("Hello");

        // Wait...
        using namespace std::chrono_literals;
        switch(auto status = hello_future.wait_for(15s); status) {
            case std::future_status::timeout:
                ASSERT_TRUE(false) << "Timed out after 15s.";
                break;
            case std::future_status::ready:
                break;
            default:
                ASSERT_TRUE(false) << "Something else went wrong";
                break;
        }
        ASSERT_TRUE(hello_future.valid());
        EXPECT_EQ(hello_future.get(), "Hello");

        ASSERT_TRUE(listener.joinable());
        listener.join();

    }

    TEST(Multithreading_Queue, AbortInSeries) {
        Queue<std::string> test_queue{};

        test_queue.push_back("Hello");
        test_queue.push_back("World");

        test_queue.abort();
        ASSERT_FALSE(test_queue.try_pop_front());
        ASSERT_FALSE(test_queue.wait_pop_front());
    }

    TEST(Multithreading_Queue, AbortInParallel) {
        Queue<std::string> test_queue;

        std::promise<bool> aborted_no_value;
        std::future<bool> anv_future = aborted_no_value.get_future();

        std::thread listener{[&]() {
            auto node = test_queue.wait_pop_front();
            aborted_no_value.set_value(node == nullptr);
        }};

        test_queue.abort();

        // Wait...
        using namespace std::chrono_literals;
        switch(auto status = anv_future.wait_for(15s); status) {
            case std::future_status::timeout:
                ASSERT_TRUE(false) << "Timed out after 15s.";
                break;
            case std::future_status::ready:
                break;
            default:
                ASSERT_TRUE(false) << "Something else went wrong";
                break;
        }
        ASSERT_TRUE(anv_future.valid());
        EXPECT_EQ(anv_future.get(), true);

        ASSERT_TRUE(listener.joinable());
        listener.join();

    }


}