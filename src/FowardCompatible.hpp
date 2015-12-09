/*
* FowardCompatible.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.12
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_FOWARDCOMPATIBLE_HPP
#define SVNSRV_FOWARDCOMPATIBLE_HPP

#include <vector>

struct HostElement {
  uint16_t begin;
  uint16_t end;
  bool IsEnabled;
  char reserved[3];
  std::string address;
};
class FowardDiscoverManager {
private:
  std::vector<HostElement> hostElement_;
  std::string defaultElement_;
  int hostPort;

public:
  bool InitializeManager(const char *tableFile);
  bool GetAddress(const std::string &owner, std::string &address);
  int GetHostPort() { return hostPort; }
};

#endif
