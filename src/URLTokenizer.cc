/*
* URLTokenizer.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include "URLTokenizer.hpp"

/**
* svn://subversion.io/subversion/subversion
**/

bool URLTokenizer(const std::string &baseURL, SubversionURL &us) {
  if (baseURL.empty())
    return false;
  int depth = 0;
  us.path.push_back('/');
  std::string str_port;
  bool bPort = false;
  for (auto &i : baseURL) {
    if (i == '/' && depth < 5) {
      depth++;
      continue;
    }
    switch (depth) {
    case 0:
      if (i == ':')
        continue;
      us.scheme.push_back(i);
      break;
    case 1:
      ////URL must svn:// not svn:/fuck/...
      return false;
      break;
    case 2:
      if (i == ':') {
        bPort = true;
        continue;
      }
      if (bPort) {
        str_port.push_back(i);
      } else {
        us.host.push_back(i);
      }
      break;
    case 3:
      us.owner.push_back(i);
      break;
    case 4:
      us.repository.push_back(i);
      break;
    case 5:
    default:
      us.path.push_back(i);
      break;
    }
    ////
  }
  // std::cout << depth << std::endl;
  if (depth <= 3)
    return false;
  if (str_port.empty()) {
    us.port = 3690;
  } else {
    char *c;
    us.port = strtol(str_port.c_str(), &c, 10);
  }
  us.origin.assign(baseURL);
  return true;
}
