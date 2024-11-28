/**
 * @file:	CoVecFut.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/27 10:56:16 Wednesday
 * @brief:
 **/

#include <chrono>
#include <coroutine>
#include <future>
#include <iostream>
#include <random>
#include <vector>

struct Task
{
    struct promise_type
    {
        Task get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception()
        {
            std::terminate();
        }
    };

    explicit Task(std::coroutine_handle<promise_type> h)
        : coro(h) {}

    ~Task()
    {
        if (coro)
            coro.destroy();
    }

    std::coroutine_handle<promise_type> coro;
};

Task pollFutures(
    std::chrono::seconds timeout,
    std::vector<std::tuple<int, std::future<int>>>& named_futures,
    std::vector<std::tuple<int, int>>& results)
{
    auto start_time = std::chrono::steady_clock::now();

    while (!named_futures.empty() &&
           std::chrono::steady_clock::now() - start_time < timeout)
    {
        for (auto it = named_futures.begin(); it != named_futures.end();)
        {
            // Check if the future is ready
            if (std::get<1>(*it).wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                // Get the result and the ID
                int idx = std::get<0>(*it);
                int result = std::get<1>(*it).get();
                std::cout << "Future result for ID " << idx << ": " << result << std::endl;
                it = named_futures.erase(it);  // Remove resolved future
                results.emplace_back(idx, result);
            }
            else
            {
                ++it;
            }
        }
        co_await std::suspend_always{};  // Yield control to simulate an event loop
    }

    // Handle unresolved futures
    for (const auto& fut : named_futures)
    {
        int idx = std::get<0>(fut);
        results.emplace_back(idx, -1);  // Store unresolved futures with status -1
    }
}

int main()
{
    // Create a vector of futures using std::async
    std::vector<std::tuple<int, std::future<int>>> named_futures;

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution distr(1, 8);

    for (int i = 0; i < 10; ++i)
    {
        int sleep_time = distr(eng);
        std::cout << "Futures cst: " << i << " -> " << sleep_time << std::endl;
        std::future<int> f = std::async(std::launch::async, [i, sleep_time]()
                                        {
									      std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
                                          return sleep_time; });
        named_futures.push_back(std::make_tuple(i, std::move(f)));
    }

    // Event loop using the coroutine
    auto timeout = std::chrono::seconds(6);
    std::vector<std::tuple<int, int>> results;
    auto task = pollFutures(timeout, named_futures, results);
    while (!task.coro.done())
    {
        task.coro.resume();  // Resume the coroutine
        std::this_thread::sleep_for(
            std::chrono::milliseconds(5));  // Simulate event loop delay
    }

    // Output unresolved results
    for (const auto& result : results)
    {
        std::cout << "Results: " << std::get<0>(result) << " -> " << std::get<1>(result) << std::endl;
    }
    return 0;
}
