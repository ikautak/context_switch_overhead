#include <chrono>
#include <iostream>
#include <vector>

#include "libaco/aco.h"

constexpr int THREAD_NUM = 100;
constexpr int YIELD_COUNT = 100'000;

void func() {
  volatile int i = 0;

  while (i < YIELD_COUNT) {
    ++i;

    aco_yield();
  }

  aco_exit();
}

int main(int argc, char *argv[]) {
  aco_thread_init(nullptr);
  aco_t *main_co = aco_create(nullptr, nullptr, 0, nullptr, nullptr);
  aco_share_stack_t *sstk = aco_share_stack_new(0);

  std::vector<aco_t *> v;
  for (int i = 0; i < THREAD_NUM; ++i) {
    v.push_back(aco_create(main_co, sstk, 0, func, nullptr));
  }

  auto start = std::chrono::system_clock::now();

  for (int i = 0; i < YIELD_COUNT; ++i) {
    for (auto &t : v) {
      aco_resume(t);
    }
  }

  aco_share_stack_destroy(sstk);
  aco_destroy(main_co);

  auto end = std::chrono::system_clock::now();
  double t = static_cast<double>(
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count());
  std::cout << "total " << t << " usec, " << t / YIELD_COUNT << " usec/yield"
            << std::endl;

  return EXIT_SUCCESS;
}
