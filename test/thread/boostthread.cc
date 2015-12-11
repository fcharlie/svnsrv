
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <boost/thread.hpp>
#include <atomic>

#if defined(_MSC_VER)
#include <windows.h>
template <typename _Type>
inline bool AtomCompareExchange(volatile _Type *t, _Type atomOld,
                                _Type atomNew) {
  return (atomOld == InterlockedCompareExchange(t, atomNew, atomOld));
}
#define AtomInterlockedExchange InterlockedExchange
#elif defined(__clang__) || defined(__GNUC__)
#define YieldProcessor sched_yield
#define AtomCompareExchange __sync_bool_compare_and_swap
#define AtomInterlockedExchange __sync_lock_test_and_set
#endif

int main() {
  std::atomic_uint index(0);
  for (auto j = 0; j < 64; j++) {
    boost::thread([&]() {
      auto t1 = std::chrono::system_clock::now();
      auto id = pthread_self();
      auto t2 = std::chrono::system_clock::now();
      std::cout << "This Thread Id: " << id << " Use Time is:\t"
                << std::chrono::duration_cast<std::chrono::microseconds>(t2 -
                                                                         t1)
                       .count()
                << " us" << std::endl;
      index += 1;
    }).detach();
  }
  while (index < 64) {
    YieldProcessor();
  }
  return 0;
}
