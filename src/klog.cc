/*
* klog.cc
*  klog Project
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <chrono>
#include "klog.h"
using namespace klogger;

static const char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *leveMessage[] = {"Debug", "Info",  "Warn",
                                    "Error", "Fatal", nullptr};

Klogger &Klogger::instance() {
  static Klogger m_klogger;
  return m_klogger;
}

Klogger::Klogger() { id = getpid(); }

Klogger::~Klogger() {
  if (logAccess != nullptr && logAccess != stdout) {
    fflush(logAccess);
    fclose(logAccess);
  }
  if (logError != nullptr && logError != stderr) {
    fflush(logError);
    fclose(logError);
  }
}

bool Klogger::task() {
  bool result = true;
  if (logError != nullptr && logError != stderr) {
    lockE.lock();
    fflush(logError);
    fclose(logError);
    std::string newFile =
        errorFile_ + std::string(".") + std::to_string(time((time_t *)NULL));
    rename(errorFile_.c_str(), newFile.c_str());
    if ((logError = fopen(errorFile_.c_str(), "a+")) == nullptr)
      result = false;
    lockE.unlock();
    log(kInfo, "move log error file success");
  }
  if (logAccess != nullptr && logAccess != stdout) {
    lockA.lock();
    fflush(logAccess);
    fclose(logAccess);
    std::string newFile =
        infoFile_ + std::string(".") + std::to_string(time((time_t *)NULL));
    rename(infoFile_.c_str(), newFile.c_str());
    if ((logAccess = fopen(infoFile_.c_str(), "a+")) == nullptr)
      result = false;
    lockA.unlock();
  }
  return result;
}

/**
ReadMe:
ATTRIBUTES
       For an explanation of the terms used in this section, see
       ┌──────────────────┬───────────────┬─────────┐
       │Interface         │ Attribute     │ Value   │
       ├──────────────────┼───────────────┼─────────┤
       │fread(), fwrite() │ Thread safety │ MT-Safe │
       └──────────────────┴───────────────┴─────────┘
       BSD Linux also Thread Safe

       Why not also use Atom Lock?
       Because, File Lock use too long time,and cannot rename Log File
****/
size_t Klogger::writerAccess(const char *buffer, size_t size) {
  lockA.lock();
  auto l = fwrite(buffer, 1, size, logAccess);
  // fflush(logAccess);
  lockA.unlock();
  return l;
}

size_t Klogger::writerError(const char *buffer, size_t size) {
  lockE.lock();
  auto l = fwrite(buffer, 1, size, logError);
  // fflush(logError);
  lockE.unlock();
  return l;
}

bool Klogger::init(const char *infoFile, const char *errorFile) {
  FILE *fp;
  if (infoFile && (fp = fopen(infoFile, "a+")) != nullptr) {
    logAccess = fp;
    infoFile_ = infoFile;
  }
  if (errorFile && (fp = fopen(errorFile, "a+")) != nullptr) {
    logError = fp;
    errorFile_ = errorFile;
  }
  return true;
}

void Klogger::fflushE() {
  fflush(logAccess);
  fflush(logError);
}

void Klogger::access(const char *fmt, ...) {
  static thread_local char buffer[4096];
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = std::localtime(&t);
  auto len = snprintf(buffer, 4095, "[%d/%d/%d %s %d:%d:%d] ",
                      (1900 + tm->tm_year), tm->tm_mon + 1, tm->tm_mday,
                      (wday[tm->tm_wday]), tm->tm_hour, tm->tm_min, tm->tm_sec);
  char *p = buffer + len;
  size_t l = 4095 - len;
  va_list ap;
  va_start(ap, fmt);
  auto ret = vsnprintf(p, l, fmt, ap);
  va_end(ap);
  auto total = len + ret;
  buffer[total] = '\n';
  writerAccess(buffer, total + 1);
}

void Klogger::log(KloggerLevel level, const char *fmt, ...) {
  static thread_local char buffer[4096];
  static thread_local size_t tid = pthread_self();
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = std::localtime(&t);
  auto len = snprintf(
      buffer, 4095, "[%s] Process: %d Thread: %zu Time: %d/%d/%d %s %d:%d:%d ",
      leveMessage[level], id, tid, (1900 + tm->tm_year), tm->tm_mon + 1,
      tm->tm_mday, wday[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
  char *p = buffer + len;
  size_t l = 4095 - len;
  va_list ap;
  va_start(ap, fmt);
  auto ret = vsnprintf(p, l, fmt, ap);
  va_end(ap);
  auto total = len + ret;
  buffer[total] = '\n';
  writerError(buffer, total + 1);
}
