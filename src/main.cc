/*
* main.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <cstring>
//#include <sys/stat.h>
#include "klog.h"
#include "version.h"
#include "svnsrv.h"
#include "Daemonize.h"
#include "TOMLParse.hpp"
#include "SubversionServer.hpp"
#include "RouterSeletor.hpp"
#include "Runtime.hpp"
#include "Argv.hpp"

enum ProcessSignalFlow {
  kRequireExit = 0,
  kKillProcessFailed = 1,
  kRequireContinue = 2,
  kArgumentError
};

static const char *kUsage =
    "OVERVIEW: svnsrv subversion proxy service\n"
    "Usage: svnsrv [options] <input> \n"
    "\nOPTIONS:\n"
    "  -D [--daemon]      run svnsrv as a daemon\n"
    "  -c [--config]      set svnsrv config file,file is toml format\n"
    "  -d [--debug]       run svnsrv as debug mode\n"
    "  -h [-?|--help]     print svnsrv usage information and exit\n"
    "  -v [--version]     print svnsrv version and exit\n"
    "  -q [--quiet]       print svnsrv version short mode\n"
    "  -s [--signal]      send a signal to svnsrv,argument: stop|restart \n";

static const char *kVersion =
    "svnsrv version " CONFIG_VERSION "\n"
    "svnsrv is authored by Force Charlie <forcemz@forcemz.net>.\n"
    "Copyright (C) 2016 OSChina.NET .All Rights Reserved.\n"
    "This software Licensed under the MTI License.\n";

int PrintVersion(bool quiet) {
  if (quiet) {
    printf("%s\n", CONFIG_VERSION);
  } else {
    printf("%s", kVersion);
  }
  return 0;
}

int PrintUsage() {
  printf("%s\n", kUsage);
  return 0;
}

int ProcessSignalArgs(const char *signame, const LauncherArgs &lanucherAgrs) {
  if (strcmp(signame, "stop") == 0) {
    return DaemonStop(lanucherAgrs.pidFile) ? kRequireExit : kKillProcessFailed;
  } else if (strcmp(signame, "restart") == 0) {
    return DaemonRestart(lanucherAgrs.pidFile) ? kRequireContinue
                                               : kKillProcessFailed;
  }
  printf("Unsupported signal: %s\n", signame);
  return kArgumentError;
}

int main(int argc, char **argv) {
  SavedArgv saveArgv(argc, argv);
  const char *short_opt = "?Dc:dhvqs:";
  int ch = 0;
  int opt_index = 0;
  const char *cfile = nullptr;
  const char *signame = nullptr;
  LauncherArgs launcherArgs;
  NetworkServerArgs networkArgs;
  bool daemon = false;
  bool debug = false;
  bool version = false;
  bool quiet = false;
  const option longopts[] = {{"daemon", no_argument, NULL, 'D'},
                             {"config", required_argument, NULL, 'c'},
                             {"debug", no_argument, NULL, 'd'},
                             {"help", no_argument, NULL, 'h'},
                             {"version", no_argument, NULL, 'v'},
                             {"quiet", no_argument, NULL, 'q'},
                             {"signal", required_argument, NULL, 's'},
                             {NULL, 0, 0, 0}};
  while ((ch = getopt_long(saveArgv.Argc(), saveArgv.Argv(), short_opt,
                           longopts, &opt_index)) != -1) {
    switch (ch) {
    case '?':
    case 'h':
      return PrintUsage();
    case 'D':
      daemon = true;
      break;
    case 'c':
      cfile = optarg;
      break;
    case 'd':
      debug = true;
      break;
    case 'v':
      version = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 's':
      signame = optarg;
      break;
    default:
      break;
    }
  }
  if (version)
    return PrintVersion(quiet);
  if (!ParseServiceProfile(cfile, networkArgs, launcherArgs)) {
    perror("svnsrv parse svnsrv.toml failed !\n");
    return -1;
  }
  if (!InitializeRouterSeletor(launcherArgs.routerFile)) {
    perror("svnsrv cannot initialized RouterSeletor !");
    return -2;
  }
  klogger::InitializeKlogger(launcherArgs.logAccess.c_str(),
                             launcherArgs.logError.c_str());
  if (signame) {
    return ProcessSignalArgs(signame, launcherArgs);
  }
  (void)debug;
  if (daemon) {
    DaemonSignalMethod();
    if (Daemonize(launcherArgs.pidFile) != 0) {
      klogger::Log(klogger::kError, "cannot create svnsrv daemon,this pid= %d",
                   GetCurrentProcessId());
      klogger::FileFlush();
      return -1;
    }
    klogger::Log(klogger::kInfo, "svnsrv run as daemon success,pid: %d",
				 GetCurrentProcessId());
    klogger::FileFlush();
    if (!DaemonWait(argc, argv, launcherArgs.allowRestart)) {
      klogger::Log(klogger::kError, "cannot create watcher process!");
      return -1;
    }
  } else {
    ForegroundSignalMethod();
  }

  return SubversionServerInitialize(networkArgs);
}
