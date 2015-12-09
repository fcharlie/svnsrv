/*
* URLTokenizer.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/

#ifndef SVNSERVICE_TOKENIZER_HPP
#define SVNSERVICE_TOKENIZER_HPP

#include <string>
#include <stdint.h>

/// url svn://subversion.io:3690/apache/subversion/trunk/src/libsvn_ra
typedef struct SubversionURL_ {
  // scheme like svn http or https
  std::string scheme;
  // like subversion.io
  std::string host;
  // like 3690
  uint32_t port;
  // like apache
  std::string owner;
  // like subversion
  std::string repository;
  // like /trunk/src/libsvn_ra
  std::string path;
  // full url
  std::string origin;
} SubversionURL;

bool URLTokenizer(const char *baseURL, SubversionURL &us);

#endif
