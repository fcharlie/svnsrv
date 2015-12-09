/*
* Daemon.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <string>
#include <fstream>
#include "klog.h"
#include "Daemon.h"
#define isEffective(c) ((c >= '0' && c <= '9') || c == ' ' || c == 0)

int CreateDaemon() {
  int fd;
  switch (fork()) {
  case -1:
    printf("fork() failed\n");
    return -1;
  case 0:
    break;
  default:
    exit(0);
  }
  if (setsid() == -1) {
    return -1;
  }

  umask(0);
  fd = open("/dev/null", O_RDWR);
  if (fd == -1) {
    // open /dev/null failed
    return -1;
  }

  if (dup2(fd, STDIN_FILENO) == -1) {
    // dup2(STDIN) failed
    return -1;
  }

  if (dup2(fd, STDOUT_FILENO) == -1) {
    // dup2(STDOUT) failed
    return -1;
  }
  if (fd > STDERR_FILENO) {
    if (close(fd) == -1) {
      return -1;
    }
  }
  return 0;
}

bool LookupDaemonPID(const std::string &pidFile, pid_t &id) {
  FILE *fp = nullptr;
  if ((fp = fopen(pidFile.c_str(), "r")) == nullptr) {
    return false;
  }
  char buffer[16] = {0};
  fread(buffer, 1, 16, fp);
  fclose(fp);
  for (auto &c : buffer) {
    if (isEffective(c))
      continue;
    return false;
  }
  char *c;
  id = (pid_t)strtol(buffer, &c, 10);
  return true;
}

bool StoreDaemonPID(const std::string &pidFile) {
  FILE *fp = nullptr;
  if ((fp = fopen(pidFile.c_str(), "w+"))) {
    return false;
  }
  fprintf(fp, "%d", getpid());
  fclose(fp);
  return true;
}

bool StopDaemonService(const std::string &pidFile) {
  ///
  return false;
}

bool RestartDaemonService(const std::string &pidFile) {
  ///
  return false;
}

void CrashHandle(const char *data, int size) {
  std::ofstream fs("/tmp/svnsrv_dump.log", std::ios::app);
  std::string str = std::string(data, size);
  fs << str;
  fs.close();
  // LOG(ERROR) << str;
}

void SIGINTMethod(int sig) {
  klogger::Log(klogger::kInfo, "Subversion Server shutdown ");
  klogger::FileFlush();
  _exit(0);
}

int BindSignal(bool isDaemon) {
  if (!isDaemon)
    signal(SIGINT, SIGINTMethod);
  return 0;
}
