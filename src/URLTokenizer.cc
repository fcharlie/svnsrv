/*
* URLTokenizer.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include "URLTokenizer.hpp"

/**
* svn://subversion.io/subversion/subversion
**/

bool URLTokenizer(const char *u, SubversionURL &us) {
  if (u == nullptr)
    return false;
  int depth = 0;
  us.path.push_back('/');
  std::string str_port;
  bool bPort=false;
  for (; *u; u++) {
    if (*u == '/') {
      depth++;
      continue;
    }
    switch (depth) {
    case 0:
      if (*u == ':')
        continue;
      us.scheme.push_back(*u);
      break;
    case 1:
      ////URL must svn:// not svn:/fuck/...
      return false;
      break;
    case 2:
      if(*u==':'){
        bPort=true;
        continue;
      }
      if(bPort){
        str_port.push_back(*u);
      }else{
        us.host.push_back(*u);
      }
      break;
    case 3:
      us.owner.push_back(*u);
      break;
    case 4:
      us.repository.push_back(*u);
      break;
    case 5:
      for (; *u; u++)
        us.path.push_back(*u);
      break;
    default:
      break;
    }
    ////
  }
  // std::cout << depth << std::endl;
  if (depth <= 3)
    return false;
  if(str_port.empty()){
    us.port=3690;
  }else{
     char *c;
     us.port=strtol(str_port.c_str(),&c,10);
  }
  us.origin.assign(u);
  return true;
}
