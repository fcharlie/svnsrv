/*
* SubversionHds.cc
* oschina.net subversion proxy service
* Subversion Protocol handshake parse
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include "SubversionHds.hpp"

/*

If the client does not support a protocol version within the specified
range, it closes the connection.  Otherwise, the client responds to
the greeting with an item matching the prototype:

  response: ( version:number ( cap:word ... ) url:string
              ? ra-client:string ( ? client:string ) )

version gives the protocol version selected by the client.  The cap
values give a list of client capabilities (see section 2.1).  url
gives the URL the client is accessing.  ra-client is a string
identifying the RA implementation, e.g. "SVN/1.6.0" or "SVNKit 1.1.4".
client is the string returned by svn_ra_callbacks2_t.get_client_string;
that callback may not be implemented, so this is optional.
*/

enum SubversionHdsState {
  kParseReset = 0, ////
  kProtocolParsed,
  kCapabilitiesParsed,
  kSubversionUrlParsed,
  kSubversionRUAParsed
};

/*
* in subversion , use svn_ra_svn__read_tuple parse network data
* proxy only parse handsake , use state machine
*/

/*
( 2 ( edit-pipeline svndiff1 absent-entries depth mergeinfo log-revprops ) \
36:svn://subversion.io/subversion/trunk \
53:SVN/1.8.13-SlikSvn-1.8.13-X64 (x64-microsoft-windows) ( ) )
*/
int ParseSubversionHds(char *buffer, std::size_t nbytes, SubversionHds &hds) {
  if (buffer == nullptr || nbytes == 0) {
    hds.lastError.assign("Malformed Network data,buffer is empty !");
    return 1;
  }
  auto p = buffer;
  std::string tmp;
  int status = kParseReset;
  auto end = buffer + nbytes;
  unsigned depth = 0;
  for (; p < end; p++) {
    switch (*p) {
    case '(':
      depth++;
      continue;
    case ')':
      depth--;
      continue;
    case ' ':
    case '\n':
      if (status == kProtocolParsed) {
        if (tmp.size()) {
          hds.mCapabilities.push_back(tmp);
          tmp.clear();
        }
      }
      continue;
    case 0:
      hds.lastError.assign("Malformed Network data");
      return 2;
    default:
      break;
    }
    switch (status) {
    case kParseReset:
      if (depth == 1) {
        hds.protocol = *p - '0';
        status = kProtocolParsed;
      }
      break;
    case kProtocolParsed:
      if (depth == 2) {
        tmp.push_back(*p);
        break;
      }
      tmp.clear();
      status = kCapabilitiesParsed;
    case kCapabilitiesParsed:
      if (*p != ':') {
        tmp.push_back(*p);
      } else {
        char *c;
        auto l = strtol(tmp.c_str(), &c, 10);
        if (p + l + 1 >= end) {
          hds.lastError.assign("Malformed Network data, URL string length mark "
                               "too long !");
          return 2;
        }
        p++;
        hds.subversionURL.assign(p, l);
        p += l;
        tmp.clear();
        status = kSubversionUrlParsed;
      }
      break;
    case kSubversionUrlParsed:
      if (depth != 1)
        status = kSubversionRUAParsed;
      if (*p != ':') {
        tmp.push_back(*p);
      } else {
        char *c;
        auto l = strtol(tmp.c_str(), &c, 10);
        if (p + l + 1 >= end) {
          hds.lastError.assign(
              "Malformed Network data, ra-client string length mark "
              "too long !");
          return 2;
        }
        p++;
        hds.userAgent.assign(p, l);
        p += l;
        tmp.clear();
        status = kSubversionRUAParsed;
      }
      break;
    case kSubversionRUAParsed:
      break;
    }
  }
  if (hds.subversionURL.empty()) {
    hds.lastError.assign(
        "Parse Subversion Handsake failed, cannot resolve url !");
    return 2;
  }
  return 0;
}

// enum CanonicalizeDomain {
//   kNormalDomain = 0,
//   kAccountDomain = 1,
//   kHostPortDomain = 2
// };

int SubversionUriCanonicalize(const std::string &url,
                              SubversionURL &subversionURL) {
  // svn://git.oschina.net/some/repo/path
  // svn://git.oschina.net:port/some/repo/path
  if (url.empty())
    return 1;
  int depth = 0;
  subversionURL.path.push_back('/');
  std::string &domRefs = subversionURL.host;
  std::string str_port;
  for (auto &c : url) {
    if (c == '/' && depth < 5) {
      depth++;
      continue;
    }
    switch (depth) {
    case 0:
      if (c != ':')
        subversionURL.scheme.push_back(c);
      break;
    case 1:
      return 2;
    case 2: {
      if (c == '@') {
        subversionURL.account.swap(subversionURL.host);
        break;
      } else if (c == ':') {
        domRefs = str_port;
      } else {
        domRefs.push_back(c);
      }
    } break;
    case 3:
      subversionURL.owner.push_back(c);
      break;
    case 4:
      subversionURL.repository.push_back(c);
      break;
    case 5:
    default:
      subversionURL.path.push_back(c);
      break;
    }
    ////
  }
  if (str_port.empty()) {
    if (subversionURL.scheme.compare("svn") == 0) {
      subversionURL.port = 3690;
    } else if (subversionURL.scheme.compare("svn+ssh") == 0) {
      subversionURL.port = 21;
    } else {
      subversionURL.port = 3690;
    }
  } else {
    char *c;
    subversionURL.port = strtol(str_port.c_str(), &c, 10);
  }
  return 0;
}
