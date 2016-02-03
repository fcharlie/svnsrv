/*
* SubversionHds.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SUBVERSION_HANDSAKE_HPP
#define SUBVERSION_HANDSAKE_HPP
#include <string>
#include <vector>
#include <cstdint>

// URL RFC http://www.ietf.org/rfc/rfc3986.txt
// //<user>:<password>@<host>:<port>/<url-path>
struct SubversionURL {
  // svn/svn+ssh
  std::string scheme;
  // like subversion.io
  std::string account;
  std::string host;
  // like 3690
  std::uint32_t port;
  // like apache
  std::string owner;
  // like subversion
  std::string repository;
  // like /trunk/src/libsvn_ra
  std::string path;
};

/*
Resources:
http://svn.apache.org/repos/asf/subversion/trunk/subversion/libsvn_ra_svn/protocol
2.1 Capabilities

The following capabilities are currently defined (S indicates a server
capability and C indicates a client capability):

[CS] edit-pipeline     Every released version of Subversion since 1.0
                       announces the edit-pipeline capability; starting
                       in Subversion 1.5, both client and server
                       *require* the other side to announce edit-pipeline.
[CS] svndiff1          If both the client and server support svndiff version
                       1, this will be used as the on-the-wire format for
                       svndiff instead of svndiff version 0.
[CS] absent-entries    If the remote end announces support for this capability,
                       it will accept the absent-dir and absent-file editor
                       commands.
[S]  commit-revprops   If the server presents this capability, it supports the
                       rev-props parameter of the commit command.
                       See section 3.1.1.
[S]  mergeinfo         If the server presents this capability, it supports the
                       get-mergeinfo command.  See section 3.1.1.
[S]  depth             If the server presents this capability, it understands
                       requested operational depth (see section 3.1.1) and
                       per-path ambient depth (see section 3.1.3).
[S]  atomic-revprops   If the server presents this capability, it
                       supports the change-rev-prop2 command.
                       See section 3.1.1.
[S]  inherited-props   If the server presents this capability, it supports the
                       retrieval of inherited properties via the get-dir and
                       get-file commands and also supports the get-iprops
                       command (see section 3.1.1).
*/

struct SubversionHds {
  std::uint32_t protocol;
  std::vector<std::string> mCapabilities;
  std::string subversionURL;
  std::string userAgent;
  std::string lastError;
};
int ParseSubversionHds(char *buffer, std::size_t nbytes, SubversionHds &hds);
int SubversionUriCanonicalize(const std::string &url,
                              SubversionURL &subversionURL);

#endif
