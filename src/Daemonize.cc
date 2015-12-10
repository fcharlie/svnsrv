/*
* Daemonize.cc
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
#include <vector>
#include "klog.h"
#include "Daemonize.h"
#define isEffective(c) ((c >= '0' && c <= '9') || c == ' ' || c == 0)

int Daemonize() {
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
  if ((fp = fopen(pidFile.c_str(), "w+")) == nullptr) {
    klogger::Log(klogger::kFatal, "cannot open %s", pidFile.c_str());
    return false;
  }
  fprintf(fp, "%d", getpid());
  fclose(fp);
  return true;
}

bool StopDaemonService(const std::string &pidFile) {
  pid_t id;
  int l = 1;
  if (LookupDaemonPID(pidFile, id)) {
    printf("kill svnsrv daemon ,pid: %d\n", id);
    l = kill(id, SIGUSR1);
  } else {
    return false;
  }
  unlink(pidFile.c_str());
  return l == 0;
}

bool RestartDaemonService(const std::string &pidFile) {
  pid_t id;
  int l = 1;
  if (LookupDaemonPID(pidFile, id)) {
    unlink(pidFile.c_str());
    l = kill(id, SIGUSR2);
  } else {
    return false;
  }
  return l == 0;
}

void CrashHandle(const char *data, int size) {
  std::ofstream fs("/tmp/svnsrv_dump.log", std::ios::app);
  std::string str = std::string(data, size);
  fs << str;
  fs.close();
  // LOG(ERROR) << str;
}

extern pid_t daemon_child;

void SignalDaemonKill(int sig) {
  klogger::Log(klogger::kInfo, "svnsrv daemon shutdown");
  klogger::FileFlush();
  _exit(0);
}
void SubversionStopCallback();
void SignalDaemonRestart(int sig) {
  klogger::Log(klogger::kInfo, "stop older svnsrv daemon socket accpter");
  SubversionStopCallback();
  char filename[32];
  snprintf(filename, 32, "/proc/%d/cmdline", getpid());
  char buffer[4096] = {0};
  auto fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open:");
    klogger::Log(klogger::kFatal, "cannot open /proc/self/cmdline");
    klogger::FileFlush();
    _exit(-1);
  }
  auto ret = read(fd, buffer, 4096);
  std::vector<char *> Argv_;
  Argv_.push_back(buffer);
  for (auto i = 0; i < ret; i++) {
    if (buffer[i] == 0 && i + 1 < ret) {
      Argv_.push_back(&buffer[++i]);
    }
  }
  klogger::Log(klogger::kInfo, "start new process, Argv size: %d,First Arg: %s",
               Argv_.size(), Argv_.at(0));
  klogger::FileFlush();
  execvp(Argv_[0], Argv_.data());
  klogger::Log(klogger::kFatal, "cannot start svnsrv");
  _exit(-1);
}

void SignalProcessKill(int sig) {
  klogger::Log(klogger::kInfo, "svnsrv shutdown");
  klogger::FileFlush();
  _exit(0);
}

int SIGINTRegister() {
  signal(SIGINT, SignalProcessKill);
  return 0;
}

int DaemonSignalMethod() {
  ///
  signal(SIGUSR1, SignalDaemonKill);
  signal(SIGUSR2, SignalDaemonRestart);
  return 0;
}
