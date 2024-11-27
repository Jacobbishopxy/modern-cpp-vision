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
#include <vector>

struct Task {
  struct promise_type {
    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() { std::terminate(); }
  };

  explicit Task(std::coroutine_handle<promise_type> h) : coro(h) {}
  ~Task() {
    if (coro) coro.destroy();
  }

  std::coroutine_handle<promise_type> coro;
};

Task pollFutures(std::vector<std::future<int>>& futures,
                 std::chrono::seconds timeout) {
  auto start_time = std::chrono::steady_clock::now();
  while (!futures.empty() &&
         std::chrono::steady_clock::now() - start_time < timeout) {
    for (auto it = futures.begin(); it != futures.end();) {
      if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        std::cout << "Future result: " << it->get() << "\n";
        it = futures.erase(it);  // Remove resolved future
      } else {
        ++it;
      }
    }
    co_await std::suspend_always{};  // Yield control to simulate an event loop
  }
  if (!futures.empty()) {
    std::cerr << "Timeout reached. Unresolved futures remain.\n";
  }
}

int main() {
  // Create a vector of futures using std::async
  std::vector<std::future<int>> futures;
  for (int i = 0; i < 5; ++i) {
    futures.push_back(std::async(std::launch::async, [i]() {
      std::this_thread::sleep_for(std::chrono::seconds(100 * (i + 1)));
      return i * i;
    }));
  }

  // Event loop using the coroutine
  auto timeout = std::chrono::seconds(5);  // Set a timeout of 600ms
  auto task = pollFutures(futures, timeout);
  while (!task.coro.done()) {
    task.coro.resume();  // Resume the coroutine
    std::this_thread::sleep_for(
        std::chrono::milliseconds(50));  // Simulate event loop delay
  }

  return 0;
}
