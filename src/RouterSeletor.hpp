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

bool InitializeRouterSeletor(const std::string &file);
bool GetStorageElement(const SubversionURL &su, StorageElement &elem);

#endif
