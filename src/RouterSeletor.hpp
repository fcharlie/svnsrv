/*
* FowardCompatible.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.12
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_FOWARDCOMPATIBLE_HPP
#define SVNSRV_FOWARDCOMPATIBLE_HPP
#include <string>
#include <vector>
#ifndef SUBVERSION_HANDSAKE_HPP
#include "SubversionHds.hpp"
#endif

struct StorageElement {
  uint16_t port;
  uint8_t reserved[2];
  std::string address;
};
struct HostElement {
  uint16_t begin;
  uint16_t end;
  uint16_t port;
  bool enabled;
  uint8_t reserved;
  std::string address;
};

class RouterSeletor {
private:
  std::vector<HostElement> hostElement_;
  std::string defaultElement_;
  uint16_t defaultPort_;

public:
  bool InitializeManager(const char *tableFile);
  bool GetAddress(const std::string &owner, StorageElement &elem);
};

bool InitializeRouterSeletor(const std::string &file);
bool GetStorageElement(const SubversionURL &su, StorageElement &elem);

#endif
