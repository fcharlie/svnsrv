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

bool Klogger::RollingFileA() {
  if (logAccess != stdout && logAccess) {
    fclose(logAccess);
    auto t =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto tm = std::localtime(&t);
    char newname[4096];
    snprintf(newname, 4096, "%s.%d-%d-%d+%d.%d", infoFile_.data(),
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_yday, tm->tm_hour,
             tm->tm_min);
    // auto newname = infoFile_ + std::to_string();
    // rename(infoFile_.data(), const char *__new)
    rename(infoFile_.data(), newname);
    logAccess = fopen(infoFile_.data(), "a+");
    if (logAccess == nullptr)
      logAccess = stdout;
  }
  return true;
}
bool Klogger::RollingFileE() {
  if (logError != stderr && logError) {
    fclose(logError);
    auto t =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto tm = std::localtime(&t);
    char newname[4096];
    snprintf(newname, 4096, "%s.%d-%d-%d+%d.%d", errorFile_.data(),
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_yday, tm->tm_hour,
             tm->tm_min);
    // auto newname = infoFile_ + std::to_string();
    // rename(infoFile_.data(), const char *__new)
    rename(errorFile_.data(), newname);
    logError = fopen(errorFile_.data(), "a+");
    if (logError == nullptr)
      logError = stdout;
  }
  return true;
}

size_t Klogger::writerAccess(const char *buffer, size_t size) {
  std::lock_guard<std::mutex> lock(mtxE);
  if (accessCounts >= MAX_LINE_NUMBER)
    this->RollingFileA();
  accessCounts++;
#ifdef _WIN32
  auto l = fwrite(buffer, 1, size, logAccess);
  fflush(logAccess);
  return l;
#else
  return fwrite(buffer, 1, size, logAccess);
#endif
}

size_t Klogger::writerError(const char *buffer, size_t size) {
  std::lock_guard<std::mutex> lock(mtxE);
  if (errorCounts >= MAX_LINE_NUMBER)
    this->RollingFileE();
  errorCounts++;
#ifdef _WIN32
  auto l = fwrite(buffer, 1, size, logError);
  fflush(logError);
  return l;
#else
  return fwrite(buffer, 1, size, logError);
#endif
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
  auto len = snprintf(buffer, 4095, "[%s] Process: %d Thread: %" PRIu64
                                    " Time: %d/%02d/%02d %s %02d:%02d:%02d ",
                      leveMessage[level], (int)id, tid, (1900 + tm->tm_year),
                      tm->tm_mon + 1, tm->tm_mday, wday[tm->tm_wday],
                      tm->tm_hour, tm->tm_min, tm->tm_sec);
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
