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
#include <string>
#include <mutex>

namespace klogger {
enum KloggerLevel {
  kDebug = 0, /// Debug
  kInfo = 1,
  kWarn = 2,
  kError = 3,
  kFatal = 4
};

class Klogger {
private:
  // pid_t id;
  std::mutex mtxA;
  std::mutex mtxE;
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
  bool init(const char *infoFile, const char *errorFile = nullptr);
  void fflushE();
  void access(const char *fmt, ...);
  void destroy(const char *msg);
  void log(KloggerLevel level, const char *fmt, ...);
  static Klogger &instance();
};
#define InitializeKlogger Klogger::instance().init ///;
#define Access Klogger::instance().access          ///;
#define Log Klogger::instance().log
#define FileFlush Klogger::instance().fflushE ////
#define Destroy Klogger::instance().destroy
#define KLOGGER_VERSION 1
}

#endif
