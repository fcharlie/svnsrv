/*
* SubversionSyntactic.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include "SubversionParse.hpp"

/*
  item   = word / number / string / list
  word   = ALPHA *(ALPHA / DIGIT / "-") space
  number = 1*DIGIT space
  string = 1*DIGIT ":" *OCTET space
         ; digits give the byte count of the *OCTET portion
  list   = "(" space *item ")" space
  space  = 1*(SP / LF)
*/

bool HandshakeInfo::Parse(char *str, size_t length) {
  if (str == nullptr || length == 0) {
    lastError.assign("Malformed Network data !,Null of buffer");
    return false;
  }
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
    case '\n':
    case ' ': {
      if (status == kCapabilities) {
        if (!cbuf.empty())
          mCapabilities.push_back(cbuf);
        cbuf.clear();
      }
    } break;
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
        if (str[i] == ' ')
          continue;
        cbuf.push_back(str[i]);
      }
    } break;
    case kSubversionURL:
      if (str[i] == ' ')
        continue;
      if (isdigit(str[i])) {
        stringLength.push_back(str[i]);
      } else {
        char *c;
        auto n = strtol(stringLength.c_str(), &c, 10);
        i++;
        if (n + i >= length) {
          lastError.assign("Bad URL Length offset");
          return false;
        }
        baseURL.assign(&str[i], n);
        i += n;
        stringLength.clear();
        status = kUserAgent;
      }
      break;
    case kUserAgent: {
      if (str[i] == ' ')
        continue;
      if (isdigit(str[i])) {
        stringLength.push_back(str[i]);
      } else {
        char *c;
        auto n = strtol(stringLength.c_str(), &c, 10);
        i++;
        if (n + i >= length) {
          lastError.assign("Bad UserAgent Length offset");
          return false;
        }
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
  if (depth != 0 || baseURL.empty()) {
    lastError.assign("Malformed Network data !");
    return false;
  }
  if (!URLTokenizer(baseURL, su)) {
    lastError.assign("Irregular spelling of URL '");
    lastError.append(baseURL);
    lastError.push_back('\'');
    return false;
  }
  return true;
}
