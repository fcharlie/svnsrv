/*
* svnsrv.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/

#ifndef MIRACLE_SVNSRV_H
#define MIRACLE_SVNSRV_H
#include <stdint.h>
#include <string>

typedef struct NetworkServerArgs__{
  uint32_t poolSize;
  uint32_t port;
  uint32_t compressionLevel;
  bool isDomainFilter;
  bool isTunnel;
  bool reserved[2];
  std::string address;
  std::string domain;
}NetworkServerArgs;


/// Align 4
typedef struct LauncherArgs__ {
  std::string pidFile;
  std::string logAccess;
  std::string logError;
} LauncherArgs;

#endif
