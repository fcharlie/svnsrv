/*
* main.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include "cpptoml.h"
#include "klog.h"
#include "version.h"
#include "svnsrv.h"
#include "Daemon.h"
#include "SubversionServer.hpp"
#include "FowardCompatible.hpp"
extern FowardDiscoverManager fowardDiscoverManager;

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
    "Copyright (C) 2015 OSChina.NET .All Rights Reserved.\n"
    "This software Licensed under the MTI License.\n";

bool GetProcessImageFileFolder(char *buffer, size_t bufSize) {
  auto sz = readlink("/proc/self/exe", buffer, bufSize - 1);
  if (sz == 0)
    return false;
  buffer[sz] = 0;
  while (--sz) {
    if (buffer[sz] == '/') {
      if (sz == 0)
        buffer[1] = 0;
      else
        buffer[sz] = 0;
      return true;
    }
  }
  buffer[1] = 0;
  return true;
}

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
    return StopDaemonService(lanucherAgrs.pidFile) ? 0 : -1;
  } else if (strcmp(signame, "restart") == 0) {
    return RestartDaemonService(lanucherAgrs.pidFile) ? 0 : -1;
  } else {
    printf("Unsupported signal: %s\n", signame);
    return 1;
  }
  return 0;
}

bool ParseServiceProfile(const char *cfile, NetworkServerArgs &na,
                         LauncherArgs &la) {
  std::string tomlfile;
  char buffer[4096];
  if (cfile == nullptr) {
    if (GetProcessImageFileFolder(buffer, 4096) == false) {
      printf("Failure, svnsrv cannot found self exe path!\n");
      return false;
    }
    tomlfile = std::string(buffer) + "/svnsrv.toml";
    if (access(tomlfile.c_str(), F_OK) != 0) {
      tomlfile = std::string(getenv("HOME")) + "/.svnsrv/svnsrv.toml";
      if (access(tomlfile.c_str(), R_OK) != 0)
        return false;
    }
  } else {
    if (access(cfile, R_OK) != 0)
      return false;
    tomlfile = cfile;
  }
  std::shared_ptr<cpptoml::table> g;
  try {
    g = cpptoml::parse_file(tomlfile);
  } catch (const cpptoml::parse_exception &e) {
    printf("Failure, Parser svnsrv.toml failed: %s \n", e.what());
    return false;
  }
  auto Strings = [&](const char *key, const char *v) -> std::string {
    if (g->contains_qualified(key)) {
      return g->get_qualified(key)->as<std::string>()->get();
    }
    return std::string(v);
  };
  auto Integer = [&](const char *key, int vi) -> int {
    if (g->contains_qualified(key)) {
      auto i64 = g->get_qualified(key)->as<int64_t>()->get();
      return static_cast<int>(i64);
    }
    return vi;
  };
  auto Boolean = [&](const char *key, bool b) -> bool {
    if (g->contains_qualified(key)) {
      return g->get_qualified(key)->as<bool>()->get();
    }
    return b;
  };
  // Network Server Args
  na.poolSize = Integer("Service.PoolSize", 64);
  na.port = Integer("Network.Port", 3690);
  na.compressionLevel = Integer("Network.Compression", 0);
  na.address = Strings("Network.Address", "127.0.0.1");
  na.domain = Strings("Network.Domain", "git.oschina.net");
  na.isDomainFilter = Boolean("Network.DomainFilter", false);
  na.isTunnel = Boolean("Network.Tunnel", false);
  ///// Launcher Args
  la.logAccess = Strings("Logger.Access", "/tmp/svnsrv.access.log");
  la.logError = Strings("Logger.Error", "/tmp/svnsrv.error.log");
  la.pidFile = Strings("Daemon.PidFile", "/tmp/svnsrv.pid");

  auto tableFile = Strings("Router.RangeFile", "router.toml");
  fowardDiscoverManager.InitializeManager(tableFile.data());

  return true;
}

int main(int argc, char **argv) {
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
  while ((ch = getopt_long(argc, argv, short_opt, longopts, &opt_index)) !=
         -1) {
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
    printf("Error:svnsrv cannot load svnsrv.toml ,please check your file: %s\n",
           cfile == nullptr ? "path/to/svnsrv/./svnsrv.toml" : cfile);
    return 1;
  }
  if (signame)
    return ProcessSignalArgs(signame, launcherArgs);
  if (debug) {
    //// do some thing
  }
  if (daemon) {
    if (CreateDaemon() != 0) {
      printf("create daemon failed \n");
      return -1;
    }
    StoreDaemonPID(launcherArgs.pidFile);
  } else {
    BindSignal(false);
  }
  klogger::InitializeKlogger(launcherArgs.logAccess.c_str(),
                             launcherArgs.logError.c_str());
  klogger::Log(klogger::kInfo, "Listener address: %s Port: %d",
               networkArgs.address.c_str(), networkArgs.port);
  return SubversionServerInitialize(networkArgs);
}
