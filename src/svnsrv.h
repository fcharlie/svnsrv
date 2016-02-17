/*
* svnsrv.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/

#ifndef MIRACLE_SVNSRV_H
#define MIRACLE_SVNSRV_H
#include <cstdint>
#include <string>

/*
* Subversion Timeout setting
*/
#define TIMEOUT_INFINITE ((uint32_t)-1)
/// if timeout equal TIMEOUT_INFINITE, not setting timeout
#define TIMEOUT_LIMIT 43200 /// timeout is 12 hours

typedef struct NetworkServerArgs__ {
  uint32_t poolSize;
  uint32_t port;
  uint32_t compressionLevel;
  uint32_t connectTimeout;
  bool isDomainFilter;
  bool isTunnel;
  uint8_t reserved[2];
  std::string address;
  std::string domain;
} NetworkServerArgs;

/// Align 4
typedef struct LauncherArgs__ {
  std::string pidFile;
  std::string logAccess;
  std::string logError;
  std::string routerFile;
  bool allowRestart;
  uint8_t reserved[3];
} LauncherArgs;

#endif
