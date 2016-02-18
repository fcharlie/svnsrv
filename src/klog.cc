/*
* klog.cc
*  klog Project
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <chrono>
#include "klog.h"
#include "Runtime.hpp"
#define __STDC_FORMAT_MACROS  
#include <inttypes.h>

using namespace klogger;

static const char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *leveMessage[] = {"Debug", "Info",  "Warn",
                                    "Error", "Fatal", nullptr};

Klogger &Klogger::instance() {
  static Klogger m_klogger;
  return m_klogger;
}

Klogger::Klogger() {}

Klogger::~Klogger() {
  if (logAccess != nullptr && logAccess != stdout) {
    // fflush(logAccess);
    fclose(logAccess);
  }
  if (logError != nullptr && logError != stderr) {
    // fflush(logError);
    fclose(logError);
  }
}

size_t Klogger::writerAccess(const char *buffer, size_t size) {
  std::lock_guard<std::mutex> lock(mtxE);
  return fwrite(buffer, 1, size, logAccess);
}

size_t Klogger::writerError(const char *buffer, size_t size) {
  std::lock_guard<std::mutex> lock(mtxE);
  return fwrite(buffer, 1, size, logError);
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

void Klogger::destroy(const char *msg) {
  char buffer[4096];
  auto id = GetCurrentProcessId();
  uint64_t tid = GetCurrentThreadId();
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = std::localtime(&t);
  auto len =
	  snprintf(buffer, 4096, "[Info] Process: %d Thread: %" PRIu64 " Time: "
                             "%d/%02d/%02d %s %02d:%02d:%02d %s\n",
               (int)id, tid, (1900 + tm->tm_year), tm->tm_mon + 1, tm->tm_mday,
               wday[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec, msg);
  if (logError) {
    fwrite(buffer, 1, len, logError);
    fclose(logError);
    logError = nullptr;
  }
  if (logAccess) {
    fclose(logAccess);
    logAccess = nullptr;
  }
}

void Klogger::access(const char *fmt, ...) {
  char buffer[4096];
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = std::localtime(&t);
  auto len = snprintf(buffer, 4095, "[%d/%02d/%02d %s %02d:%02d:%02d] ",
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
  char buffer[4096];
  uint64_t tid = GetCurrentThreadId();
  auto id = GetCurrentProcessId();
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  auto tm = std::localtime(&t);
  auto len = snprintf(
      buffer, 4095,
	  "[%s] Process: %d Thread: %" PRIu64 " Time: %d/%02d/%02d %s %02d:%02d:%02d ",
      leveMessage[level], (int)id, tid, (1900 + tm->tm_year), tm->tm_mon + 1,
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
