/**
 * @file:	AsyncIO.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/26 22:54:43 Tuesday
 * @brief:
 **/

#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

// Simple awaitable type for simulating asynchronous I/O
struct AsyncIO {
  struct promise_type {
    std::queue<std::string> messages;  // Simulated message queue
    std::mutex mtx;
    std::condition_variable cv;

    AsyncIO get_return_object() {
      return AsyncIO{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    auto initial_suspend() { return std::suspend_never{}; }

    auto final_suspend() noexcept { return std::suspend_always{}; }

    void return_void() {}

    void unhandled_exception() { std::terminate(); }

    struct Awaitable {
      promise_type& promise;

      bool await_ready() noexcept {
        std::lock_guard lock(promise.mtx);
        return !promise.messages.empty();
      }

      void await_suspend(std::coroutine_handle<>) {
        // Wait for a new message
        std::unique_lock lock(promise.mtx);
        promise.cv.wait(lock, [&] { return !promise.messages.empty(); });
      }

      std::string await_resume() {
        // Retrieve the next message
        std::lock_guard lock(promise.mtx);
        auto msg = promise.messages.front();
        promise.messages.pop();
        return msg;
      }
    };

    Awaitable yield_value(std::string msg) {
      std::lock_guard lock(mtx);
      messages.push(msg);
      cv.notify_one();  // Notify that a message is available
      return Awaitable{*this};
    }
  };

  std::coroutine_handle<promise_type> handle;

  AsyncIO(std::coroutine_handle<promise_type> h) : handle(h) {}

  ~AsyncIO() {
    if (handle) handle.destroy();
  }

  AsyncIO(const AsyncIO&) = delete;
  AsyncIO& operator=(const AsyncIO&) = delete;

  AsyncIO(AsyncIO&& other) noexcept : handle(other.handle) {
    other.handle = nullptr;
  }

  AsyncIO& operator=(AsyncIO&& other) noexcept {
    if (this != &other) {
      if (handle) handle.destroy();
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  bool ready() const {
    std::lock_guard lock(handle.promise().mtx);
    return !handle.promise().messages.empty();
  }

  std::string get_message() {
    auto msg = handle.promise().messages.front();
    handle.promise().messages.pop();
    return msg;
  }
};

// Mock I/O listener coroutine
AsyncIO io_listener() {
  co_yield "Message 1 from I/O";
  std::this_thread::sleep_for(std::chrono::seconds(2));
  co_yield "Message 2 from I/O";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  co_yield "Final message from I/O";
}

int main() {
  auto listener = io_listener();

  while (listener.ready() || !listener.handle.done()) {
    if (!listener.ready()) {
      std::cout << "Waiting for I/O event..." << std::endl;
      listener.handle.resume();
    }
    if (listener.ready()) {
      std::cout << "Received: " << listener.get_message() << std::endl;
    }
  }

  std::cout << "All messages processed. Exiting." << std::endl;
  return 0;
}
