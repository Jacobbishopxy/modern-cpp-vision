/**
 * @file:	CoVecFut.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/26 23:33:46 Tuesday
 * @brief:
 **/
#include <chrono>
#include <coroutine>
#include <iostream>
#include <thread>
#include <vector>

struct Task {
  struct promise_type {
    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
  };

  std::coroutine_handle<promise_type> handle;

  Task(std::coroutine_handle<promise_type> h) : handle(h) {}
  ~Task() {
    if (handle) handle.destroy();
  }

  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  Task(Task&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
  Task& operator=(Task&& other) noexcept {
    if (this != &other) {
      if (handle) handle.destroy();
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }
};

// Simulated asynchronous task
Task async_task(int id, int delay_ms, std::vector<int>& results) {
  std::this_thread::sleep_for(
      std::chrono::milliseconds(delay_ms));  // Simulate async delay
  results.push_back(id * 10);                // Simulated result
  co_return;
}

// Process tasks with a timeout
void process_tasks_with_coroutines(std::vector<Task>& tasks,
                                   std::vector<int>& results,
                                   std::chrono::seconds timeout) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time = start_time + timeout;

  // Process tasks (resume coroutines)
  for (size_t i = 0; i < tasks.size(); ++i) {
    // Check if the timeout is reached before resuming the coroutine
    auto remaining_time = end_time - std::chrono::steady_clock::now();
    if (remaining_time <= std::chrono::seconds(0)) {
      // Timeout reached, fill the remaining tasks with -1
      results.push_back(-1);
    } else {
      tasks[i].handle.resume();
    }
  }

  // Fill remaining tasks with -1 if they didn't complete
  for (size_t i = tasks.size(); i < results.size(); ++i) {
    results.push_back(-1);
  }
}

int main() {
  constexpr size_t num_tasks = 1000;  // Large number of tasks
  std::vector<int> results;           // To store task results

  // Launch tasks
  std::vector<Task> tasks;
  for (size_t i = 0; i < num_tasks; ++i) {
    tasks.push_back(async_task(i, (i % 10) * 10, results));
  }

  // Process tasks with a timeout of 5 seconds
  process_tasks_with_coroutines(tasks, results, std::chrono::seconds(5));

  // Print results
  std::cout << "Collected results (up to timeout):\n";
  for (const auto& result : results) {
    std::cout << result << " ";
  }
  std::cout << std::endl;

  return 0;
}
