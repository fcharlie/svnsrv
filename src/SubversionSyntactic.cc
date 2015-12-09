/*
* SubversionSyntactic.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include "SubversionSyntactic.hpp"

// bool SubversionParser::Parse(char *s,size_t l)
// {
// 	return true;
// }

bool ExchangeCapabilities::Parse(char *str, size_t length) {
  if (str == nullptr || length == 0)
    return false;
  int depth = 0;
  size_t i = 0;
  auto status = kStatusClear;
  std::string cbuf;
  bool discoverCapabilities = false;
  std::string stringLength;
  for (; i < length; i++) {
    switch (str[i]) {
    case '(':
      depth++;
      if (depth == 2)
        discoverCapabilities = true;
      continue;
    case ')':
      depth--;
      continue;
    case ' ': {
      if (status == kCapabilities) {
        if (!cbuf.empty())
          mCapabilities.push_back(cbuf);
        cbuf.clear();
      }
    } break;
    case '\n':
      continue;
    default:
      break;
    }
    switch (status) {
    case kStatusClear:
      if (depth == 1)
        status = kProtocolVersion;
      break;
    case kProtocolVersion:
      if (isdigit(str[i]))
        protocolVersion = str[i] - '0';
      status = kCapabilities;
      break;
    case kCapabilities: {
      if (discoverCapabilities && depth == 1) {
        status = kSubversionURL;
      } else {
        cbuf.push_back(str[i]);
      }
    } break;
    case kSubversionURL:
      if (isdigit(str[i])) {
        stringLength.push_back(str[i]);
      } else {
        char *c;
        auto n = strtol(stringLength.c_str(), &c, 10);
        i++;
        if (n + i >= length)
          return false;
        baseURL.assign(&str[i], n);
        i += n;
        stringLength.clear();
        status = kUseAgent;
      }
      break;
    case kUseAgent: {
      if (isdigit(str[i])) {
        stringLength.push_back(str[i]);
      } else {
        char *c;
        auto n = strtol(stringLength.c_str(), &c, 10);
        i++;
        if (n + i >= length)
          return false;
        userAgent.assign(&str[i], n);
        i += n;
        status = kTokenEnd;
      }
    } break;
    case kTokenEnd:
    default:
      break;
    }
  }
  //  response: ( version:number ( cap:word ... ) url:string
  //               ? ra-client:string ( ? client:string ) )

  // version gives the protocol version selected by the client.  The cap
  // values give a list of client capabilities (see section 2.1).  url
  // gives the URL the client is accessing.  ra-client is a string
  // identifying the RA implementation, e.g. "SVN/1.6.0" or "SVNKit 1.1.4".
  // client is the string returned by svn_ra_callbacks2_t.get_client_string;
  // that callback may not be implemented, so this is optional.
  if (depth != 0 || status < kUseAgent)
    return false;
  return URLTokenizer(baseURL.c_str(), su);
}
