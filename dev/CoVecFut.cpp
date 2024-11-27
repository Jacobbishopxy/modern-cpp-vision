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

Task pollFutures(std::vector<std::future<std::pair<int, int>>>& futures,
                 std::chrono::seconds timeout) {
  auto start_time = std::chrono::steady_clock::now();
  while (!futures.empty() &&
         std::chrono::steady_clock::now() - start_time < timeout) {
    for (auto it = futures.begin(); it != futures.end();) {
      if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        auto [idx, sleep_time] = it->get();
        std::cout << "Future result: " << idx << " -> " << sleep_time
                  << std::endl;
        it = futures.erase(it);  // Remove resolved future
      } else {
        ++it;
      }
    }
    co_await std::suspend_always{};  // Yield control to simulate an event loop
  }
  if (!futures.empty()) {
    std::cerr << "Timeout reached. Unresolved futures remain: "
              << futures.size() << std::endl;
  }
}

int main() {
  // Create a vector of futures using std::async
  std::vector<std::future<std::pair<int, int>>> futures;

  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution distr(1, 8);

  for (int i = 0; i < 10; ++i) {
    int sleep_time = distr(eng);
    std::cout << "Futures cst: " << i << " -> " << sleep_time << std::endl;
    futures.push_back(std::async(std::launch::async, [i, sleep_time]() {
      std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
      return std::make_pair(i, sleep_time);
    }));
  }

  // Event loop using the coroutine
  auto timeout = std::chrono::seconds(6);
  auto task = pollFutures(futures, timeout);
  while (!task.coro.done()) {
    task.coro.resume();  // Resume the coroutine
    std::this_thread::sleep_for(
        std::chrono::milliseconds(5));  // Simulate event loop delay
  }

  return 0;
}
