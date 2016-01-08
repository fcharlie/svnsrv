/*
* SubversionStorage.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include "SubversionStorage.hpp"
#include "FowardCompatible.hpp"
extern FowardDiscoverManager fowardDiscoverManager;

bool DiscoverStorageNode(const SubversionURL &su, SubversionStorageNode &node) {
  node.port = fowardDiscoverManager.GetHostPort();
  return fowardDiscoverManager.GetAddress(su.owner, node.address);
}
