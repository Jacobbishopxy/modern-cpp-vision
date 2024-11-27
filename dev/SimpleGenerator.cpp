/**
 * @file:	SimpleGenerator.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/26 22:27:19 Tuesday
 * @brief:
 **/

#include <coroutine>
#include <exception>
#include <iostream>

// Define a Generator class
// Wraps the coroutine handle and provides an interface for consuming values.
template <typename T>
struct Generator {
  // manage coroutine's state
  struct promise_type {
    T current_value;

    // Required functions for the promise type
    auto get_return_object() { return Generator{this}; }

    auto initial_suspend() { return std::suspend_always{}; }

    auto final_suspend() noexcept { return std::suspend_always{}; }

    void unhandled_exception() { std::terminate(); }

    auto yield_value(T value) {
      current_value = value;
      return std::suspend_always{};
    }

    void return_void() {}
  };

  // Generator internals
  using Handle = std::coroutine_handle<promise_type>;

  explicit Generator(promise_type* p) : handle(Handle::from_promise(*p)) {}

  ~Generator() {
    if (handle) handle.destroy();
  }

  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;

  Generator(Generator&& other) noexcept : handle(other.handle) {
    other.handle = nullptr;
  }

  Generator& operator=(Generator&& other) noexcept {
    if (this != &other) {
      if (handle) handle.destroy();
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  // Iterator functions
  T next() {
    handle.resume();
    if (handle.done()) throw std::out_of_range("Generator finished");
    return handle.promise().current_value;
  }

 private:
  Handle handle;
};

// Example coroutine using the Generator
Generator<int> generate_numbers(int start, int end) {
  for (int i = start; i <= end; ++i) {
    // Pauses the coroutine and returns the current value to the caller.
    co_yield i;  // Yield each number
  }
}

int main() {
  auto gen = generate_numbers(1, 5);
  try {
    while (true) {
      std::cout << gen.next() << " ";
    }
  } catch (const std::out_of_range&) {
    std::cout << "\nGenerator completed!\n";
  }
  return 0;
}
