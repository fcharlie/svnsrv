/*
* klog.h
*  klog Project
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef MIRACLE_KLOG_H
#define MIRACLE_KLOG_H
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
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

namespace klogger {
enum KloggerLevel {
  kDebug = 0, /// Debug
  kInfo = 1,
  kWarn = 2,
  kError = 3,
  kFatal = 4
};
class AtomicLock {
private:
  volatile long mask;

public:
  AtomicLock() : mask(0) {}
  void lock() {
    while (!AtomCompareExchange(&mask, 0, 1))
      YieldProcessor();
  }
  void unlock() { AtomInterlockedExchange(&mask, 0); }
  bool trylock() {
    if (AtomCompareExchange(&mask, 0, 1))
      return true;
    return false;
  }
};
class Klogger {
private:
  // pid_t id;
  AtomicLock lockA;
  AtomicLock lockE;
  std::string infoFile_;
  std::string errorFile_;
  FILE *logAccess = stdout;
  FILE *logError = stdout;
  Klogger();
  size_t writerAccess(const char *buffer, size_t size);
  size_t writerError(const char *buffer, size_t size);

public:
  Klogger &operator=(const Klogger &) = delete;
  Klogger(Klogger &) = delete;
  ~Klogger();
  bool task();
  bool init(const char *infoFile, const char *errorFile = nullptr);
  void access(const char *fmt, ...);
  void fflushE();
  void shutdown();
  void log(KloggerLevel level, const char *fmt, ...);
  static Klogger &instance();
};
#define InitializeKlogger Klogger::instance().init ///;
#define Access Klogger::instance().access          ///;
#define Log Klogger::instance().log
#define Move Klogger::instance().task
#define FileFlush Klogger::instance().fflushE ////
#define Shutdown Klogger::instance().shutdown
#define KLOGGER_VERSION 1
}

#endif
