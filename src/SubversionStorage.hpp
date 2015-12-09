/*
* SubversionStorage.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_SUBVERSION_BACKEND_HPP
#define SVNSRV_SUBVERSION_BACKEND_HPP

#include <string>
#include "URLTokenizer.hpp"

struct SubversionStorageNode {
  int port;
  std::string address;
};

bool DiscoverStorageNode(const SubversionURL &su, SubversionStorageNode &node);

#endif
