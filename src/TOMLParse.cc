/*
* TOMLParse.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include "svnsrv.h"
#include "cpptoml.h"
#include "Runtime.hpp"

#ifdef _WIN32
#include <windows.h>
class CharacterFlip {
private:
  char *str = nullptr;

public:
  CharacterFlip(const char *u8) {
    if (u8 == nullptr)
      return;
    // to wide char
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, u8, -1, NULL, 0);
    if (unicodeLen == 0)
      return;
    wchar_t *wstr = new wchar_t[unicodeLen + 1];
    if (wstr == nullptr)
      return;
    wstr[unicodeLen] = 0;
    ::MultiByteToWideChar(CP_UTF8, 0, u8, -1, (LPWSTR)wstr, unicodeLen);

    // to codepage
    int iTextLen =
        WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    str = new char[iTextLen + 1];
    if (str == nullptr) {
      delete[] wstr;
      return;
    }
    str[iTextLen] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, iTextLen, NULL, NULL);
    delete[] wstr;
  }
  ~CharacterFlip() {
    if (str)
      delete[] str;
  }
  const char *Get() {
    if (str == nullptr)
      return nullptr;
    return const_cast<const char *>(str);
  }
};

#endif

bool ParseServiceProfile(const char *configfile, NetworkServerArgs &na,
                         LauncherArgs &la) {
  std::string tomlfile;
  if (configfile == nullptr) {
    if (!GetProcessImageFileFolder(tomlfile)) {
      perror("Failure, svnsrv cannot found self exe path!\n");
      return false;
    }
    tomlfile += "/svnsrv.toml";
    if (!PathFileIsExists(tomlfile)) {
      /// cannot found svnsrv.toml from default
      if (PathFileIsExists("svnsrv.toml")) {
        // current directory svnsrv.toml
        tomlfile = "svnsrv.toml";
      } else {
        if (!PathCombineHomeExists(tomlfile, ".svnsrv/svnsrv.toml")) {
          fprintf(stderr, "cannot found any profile in search directory !\n");
          return false;
        }
      }
    }
  } else {
    if (!PathFileIsExists(configfile)) {
      fprintf(stderr, "svnsrv custom profile: %s,not exists\n", configfile);
      return false;
    }
    tomlfile = configfile;
  }
  std::shared_ptr<cpptoml::table> g;
  try {
    g = cpptoml::parse_file(tomlfile);
  } catch (const cpptoml::parse_exception &e) {
    fprintf(stderr, "Failure, Parser svnsrv.toml failed: %s \n", e.what());
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
  /*
  *
  * TOML is case sensitive.
  * A TOML file must contain only UTF-8 encoded Unicode characters.
  * Whitespace means tab (0x09) or space (0x20).
  * Newline means LF (0x0A) or CRLF (0x0D0A).
  *-------------------------------------------------
  *
  * UTF8: https://en.wikipedia.org/wiki/UTF-8
  * UTF-8 uses the codes 0-127 only for the ASCII characters
  * ip address is complete, domain not support other encoding
  */
  na.poolSize = Integer("Service.PoolSize", 64);
  na.port = Integer("Network.Port", 3690);
  na.compressionLevel = Integer("Network.Compression", 0);
  na.connectTimeout = Integer("Network.Timeout", TIMEOUT_INFINITE);
  na.address = Strings("Network.Address", "127.0.0.1");
  na.domain = Strings("Network.Domain", "git.oschina.net");
  na.isDomainFilter = Boolean("Network.DomainFilter", false);
  na.isTunnel = Boolean("Network.Tunnel", false);

#ifdef _WIN32
  //
  auto tableFile = Strings("Router.RangeFile", "router.toml");
  CharacterFlip charTablefile(tableFile.c_str());
  la.routerFile = charTablefile.Get();

  auto accessLogFile = Strings("Logger.Access", "svnsrv.access.log");
  CharacterFlip charAccessFile(accessLogFile.c_str());
  la.logAccess = charAccessFile.Get();

  auto errorLogFile = Strings("Logger.Error", "svnsrv.error.log");
  CharacterFlip charErrorFile(errorLogFile.c_str());
  la.logError = charErrorFile.Get();

  auto pidFile = Strings("Daemon.PidFile", "svnsrv.pid");
  CharacterFlip charPidFile(pidFile.c_str());
  la.pidFile = charPidFile.Get();

  la.crashRestart = Boolean("Daemon.CrashRestart", false);
#else
  la.logAccess = Strings("Logger.Access", "/tmp/svnsrv.access.log");
  la.logError = Strings("Logger.Error", "/tmp/svnsrv.error.log");
  la.pidFile = Strings("Daemon.PidFile", "/tmp/svnsrv.pid");
  la.crashRestart = Boolean("Daemon.CrashRestart", true);
  la.routerFile = Strings("Router.RangeFile", "router.toml");
#endif
  return true;
}
