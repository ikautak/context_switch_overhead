#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "abt.h"

constexpr int THREAD_NUM = 100;
constexpr int YIELD_COUNT = 100'000;

void func(void* arg) {
  volatile int i = 0;

  while (i < YIELD_COUNT) {
    ++i;

    ABT_thread_yield();
  }
}

int main(int argc, char* argv[]) {
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(0, &cpu_set);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set)) {
    std::cerr << "setaffinity error" << std::endl;
    return EXIT_FAILURE;
  }

  ABT_xstream xstream;
  ABT_pool pool;
  std::vector<ABT_thread> threads(THREAD_NUM);

  ABT_init(argc, argv);
  ABT_xstream_self(&xstream);
  ABT_xstream_create(ABT_SCHED_NULL, &xstream);
  ABT_xstream_get_main_pools(xstream, 1, &pool);

  auto start = std::chrono::system_clock::now();

  for (int i = 0; i < THREAD_NUM; ++i) {
    ABT_thread_create(pool, func, nullptr, ABT_THREAD_ATTR_NULL, &threads[i]);
  }

  ABT_thread_join_many(THREAD_NUM, &threads[0]);

  auto end = std::chrono::system_clock::now();
  double t = static_cast<double>(
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count());
  std::cout << "total " << t << " usec, " << t / YIELD_COUNT << " usec/yield"
            << std::endl;

  return EXIT_SUCCESS;
}
