//
//
//
//
#ifndef ATOM_SAFE_TEST_H
#define ATOM_SAFE_TEST_H
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
#endif
