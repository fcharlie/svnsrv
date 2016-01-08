/*
* Daemonize.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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

class ProcessIdMask {
private:
  char *ptr = nullptr;

public:
  ProcessIdMask() {}
  bool Cache(const char *file) {
    if (file == nullptr)
      return false;
    auto l = strlen(file);
    ptr = (char *)malloc(l + 1);
    if (ptr == nullptr)
      return false;
    memcpy(ptr, file, l + 1);
    if (ptr)
      return true;
    return false;
  }
  void Release() {
    if (ptr)
      free(ptr);
  }
  const char *Get() { return ptr; }
  ~ProcessIdMask() {
    if (ptr)
      free(ptr);
  }
};

static ProcessIdMask mask;

static int check_pid(const char *pidfile) {
  int pid = 0;
  FILE *fp = fopen(pidfile, "r");
  if (fp == nullptr)
    return 0;
  int n = fscanf(fp, "%d", &pid);
  fclose(fp);
  if (n != 1 || pid == 0 || pid == getpid()) {
    return 0;
  }
  if (kill(pid, 0) && errno == ESRCH)
    return 0;
  return pid;
}

static bool write_pid(const char *pidfile) {
  FILE *fp = nullptr;
  if ((fp = fopen(pidfile, "w+")) == nullptr) {
    klogger::Log(klogger::kFatal, "cannot open %s", pidfile);
    return false;
  }
  fprintf(fp, "%d", getpid());
  fclose(fp);
  return true;
}

int Daemonize(const std::string &pidfile) {
  auto pid = check_pid(pidfile.c_str());
  if (pid != 0) {
    fprintf(stderr, "svnsrv is already running, daemon pid = %d.\n", pid);
    return 1;
  }
  int fd;
  switch (fork()) {
  case -1:
    perror("svnsrv fork() failed\n");
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
    fprintf(stderr, "cannot open /dev/null \n");
    return -1;
  }

  if (dup2(fd, STDIN_FILENO) == -1) {
    // dup2(STDIN) failed
    fprintf(stderr, "cannot dup2() set STDIN_FILENO");
    return -1;
  }

  if (dup2(fd, STDOUT_FILENO) == -1) {
    // dup2(STDOUT) failed
    fprintf(stderr, "cannot dup2() set STDOUT_FILENO");
    return -1;
  }
  if (fd > STDERR_FILENO) {
    if (close(fd) == -1) {
      return -1;
    }
  }
  if (!write_pid(pidfile.c_str())) {
    klogger::Log(klogger::kFatal, "svnsrv daemon create pidfile failed !");
    klogger::FileFlush();
    return 2;
  }
  mask.Cache(pidfile.c_str());
  return 0;
}

////////////////////////////////////////////////////////////////////
/// Daemon Action
// stop
void SignalDaemonKill(int sig) {
  if (mask.Get()) {
    unlink(mask.Get());
  }
  klogger::Shutdown();
  // klogger::Log(klogger::kInfo, "svnsrv daemon shutdown");
  // klogger::FileFlush();
  _exit(0);
}

////////// Ctrl+C
void ExitSelfEvent(int sig) {
  klogger::Shutdown();
  // klogger::Log(klogger::kInfo, "svnsrv shutdown");
  // klogger::FileFlush();
  _exit(0);
}

/////// Register signal
int SignalINTActive() {
  signal(SIGINT, ExitSelfEvent);
  return 0;
}

int DaemonSignalMethod() {
  ///
  signal(SIGUSR1, SignalDaemonKill);
  // signal(SIGUSR2, SignalDaemonRestart);
  return 0;
}

bool DaemonStop(const std::string &pidFile) {
  int l = 1;
  auto pid = check_pid(pidFile.c_str());
  if (pid == 0) {
    perror("svnsrv daemon not runing !\n");
  } else {
    int status;
    l = kill(pid, SIGUSR1);
    waitpid(pid, &status, 0);
    printf("stop svnsrv daemon success !\n");
  }
  return l == 0;
}

static bool ParsePathAndArgv(pid_t pid, std::string &path, char *buf,
                             size_t bufsize, std::vector<char *> &Argv_) {
  char filename[32];
  snprintf(filename, 32, "/proc/%d/exe", pid);
  char exePath[4096] = {0};
  auto sz = readlink(filename, exePath, 4095);
  if (sz == 0) {
    return false;
  }
  path.assign(exePath);
  snprintf(filename, 32, "/proc/%d/cmdline", pid);
  auto fd = open(filename, O_RDONLY);
  if (fd < 0) {
    klogger::Log(klogger::kFatal, "cannot open /proc/%d/cmdline", pid);
    klogger::FileFlush();
    return false;
  }
  auto ret = read(fd, buf, bufsize);
  Argv_.push_back(buf);
  for (auto i = 0; i < ret; i++) {
    if (buf[i] == 0 && i + 1 < ret) {
      Argv_.push_back(&buf[++i]);
    }
  }
  Argv_.push_back(nullptr);
  close(fd);
  return true;
}

bool DaemonRestart(const std::string &pidFile) {
  auto pid = check_pid(pidFile.c_str());
  if (pid == 0) {
    perror("svnsrv daemon not runing !\n");
    return false;
  }
  std::string path;
  char buffer[4096] = {0};
  std::vector<char *> Argv_;
  if (!ParsePathAndArgv(pid, path, buffer, 4096, Argv_)) {
    perror("cannot parse svnsrv daemon cmdline !\n");
    return false;
  }
  kill(pid, SIGUSR1);
  int status;
  waitpid(pid, &status, 0);
  int cpid = 0;
  switch ((cpid = fork())) {
  case 0: {
    auto hr = execvp(path.c_str(), Argv_.data());
    fprintf(stderr, "cannot start svnsrv,result: %d, errno: %d", hr, errno);
    klogger::Log(klogger::kFatal, "cannot start svnsrv,result: %d, errno: %s",
                 hr, strerror(errno));
    exit(-1);
  } break;
  case -1:
    break;
  default:
    waitpid(cpid, &status, 0);
    break;
  }
  auto nid = check_pid(pidFile.c_str());
  if (nid != 0) {
    printf("New daemon service runing, pid= %d\n", nid);
    return true;
  }
  return false;
}
