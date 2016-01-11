/*
* SubversionSyntactic.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_SUBVERSION_SYNTACTIC_HPP
#define SVNSRV_SUBVERSION_SYNTACTIC_HPP

#include <vector>
#include "URLTokenizer.hpp"

#define isSpace(c) (c == ' ' || c == '\n')

class ExchangeCapabilities {
private:
  enum ParseStatus : int {
    kStatusClear = 0,
    kProtocolVersion = 1,
    kCapabilities = 2,
    kSubversionURL = 3,
    kUserAgent = 4,
    kTokenEnd = 5
  };
  int protocolVersion;
  std::string baseURL;
  std::string userAgent;
  std::string lastError;
  SubversionURL su;

public:
  ExchangeCapabilities() : baseURL(256, '\0') {}
  std::vector<std::string> mCapabilities;
  bool Parse(char *s, size_t length);
  int getProtocolVersion() { return this->protocolVersion; }
  const std::string &getBaseURL() { return this->baseURL; }
  const std::string &getUserAgent() { return this->userAgent; }
  const std::string &getLastErrorMessage() { return this->lastError; }
  const SubversionURL &getSubversionURL() { return this->su; }
};

#endif
