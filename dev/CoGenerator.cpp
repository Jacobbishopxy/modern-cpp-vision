/**
 * @file:	CoGenerator.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/27 10:38:09 Wednesday
 * @brief:
 **/

#include <coroutine>
#include <future>
#include <iostream>
#include <vector>

// A simple coroutine-compatible generator for C++20
template <typename T>
struct Generator {
  struct promise_type {
    T value;
    std::suspend_always yield_value(T v) noexcept {
      value = v;
      return {};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() { std::terminate(); }
    Generator get_return_object() {
      return Generator{Handle::from_promise(*this)};
    }
  };
  using Handle = std::coroutine_handle<promise_type>;
  explicit Generator(Handle h) : coro(h) {}
  ~Generator() {
    if (coro) coro.destroy();
  }
  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;
  Generator(Generator&& other) noexcept : coro(other.coro) {
    other.coro = nullptr;
  }
  Generator& operator=(Generator&& other) noexcept {
    if (this != &other) {
      if (coro) coro.destroy();
      coro = other.coro;
      other.coro = nullptr;
    }
    return *this;
  }
  struct Iterator {
    Handle coro;
    bool operator!=(std::default_sentinel_t) const { return !coro.done(); }
    void operator++() { coro.resume(); }
    T operator*() const { return coro.promise().value; }
  };
  Iterator begin() {
    coro.resume();
    return Iterator{coro};
  }
  std::default_sentinel_t end() { return {}; }

 private:
  Handle coro;
};

Generator<int> collectFutures(std::vector<std::future<int>>& futures) {
  for (auto& future : futures) {
    co_yield future.get();  // Await the future result
  }
}

int main() {
  // Create a vector of futures using std::async
  std::vector<std::future<int>> futures;
  for (int i = 0; i < 5; ++i) {
    futures.push_back(std::async(std::launch::async, [i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
      return i * i;
    }));
  }

  // Use the coroutine to collect values
  for (auto value : collectFutures(futures)) {
    std::cout << "Future result: " << value << "\n";
  }

  return 0;
}
