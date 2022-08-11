#include <pthread.h>
#include <sched.h>
#include <syscall.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

constexpr int THREAD_NUM = 100;
constexpr int YIELD_COUNT = 100'000;

void func() {
  volatile int i = 0;

  while (i < YIELD_COUNT) {
    ++i;

    std::this_thread::yield();
  }
}

int main(int argc, char *argv[]) {
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(0, &cpu_set);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set)) {
    std::cerr << "setaffinity error" << std::endl;
    return EXIT_FAILURE;
  }

  auto start = std::chrono::system_clock::now();

  std::vector<std::thread> v;
  for (int i = 0; i < THREAD_NUM; ++i) {
    v.emplace_back(func);
  }

  for (auto &t : v) {
    t.join();
  }

  auto end = std::chrono::system_clock::now();
  double t = static_cast<double>(
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count());
  std::cout << "total " << t << " usec, " << t / YIELD_COUNT << " usec/yield"
            << std::endl;

  return EXIT_SUCCESS;
}