//
//
//
#include <iostream>
#include "SubversionHds.cc"

/*
struct SubversionHds {
  std::uint32_t protocol;
  std::vector<std::string> mCapabilities;
  std::string subversionURL;
  std::string userAgent[2];
  std::string lastError;
};
*/
void SubversionHdsPrint(const SubversionHds &hds) {
  std::cout << "Protocol: " << hds.protocol;
  std::cout << "\nCapabilities: \n[\n";
  for (auto &c : hds.mCapabilities) {
    std::cout << "\t" << c << "\n";
  }
  std::cout << "]\nSubversionURL: " << hds.subversionURL;
  std::cout << "\nUserAgent: "
            << (hds.userAgent.empty() ? "unkonw ra-client " : hds.userAgent)
            << std::endl;
}

/*
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

*/

void SubversionUrlFormat(SubversionURL &url) {
  std::cout << "Scheme: " << url.scheme << "\nAccount: "
            << (url.account.empty() ? "Anonymous" : url.account)
            << "\nHost: " << url.host << "\nPort: " << url.port
            << "\nOwner: " << url.owner << "\nRepository: " << url.repository
            << "\nPath: " << url.path << std::endl;
}

int ParseS(char *buffer, size_t len) {
  SubversionHds hds;
  SubversionURL url;
  if (ParseSubversionHds(buffer, len, hds) == 0) {
    SubversionHdsPrint(hds);
    if (SubversionUriCanonicalize(hds.subversionURL, url) == 0) {
      SubversionUrlFormat(url);
    } else {
      std::cout << "Parse Subversion URL Failed\n\n" << std::endl;
    }
  } else {
    std::cout << "--->\nLastError: " << hds.lastError << std::endl;
  }
  return 1;
}

int main() {
  char kHeader[] =
      "( 2 ( edit-pipeline svndiff1 absent-entries depth mergeinfo log-revprops"
      ") "
      "65:svn://subversion.io/subversion/subversion/trunk/sources/libsvn_ra "
      "53:SVN/1.8.13-SlikSvn-1.8.13-X64 (x64-microsoft-windows) ( ) )";

  char kHeader2[] = "( 2 ( edit-pipeline svndiff1 absent-entries depth "
                    "mergeinfo log-revprops ) "
                    "37:svn://subversion.io/apache/subversion ) ";
  char kHeaderE[] = "( 2 ( edit-pipeline svndiff1 absent-entries depth "
                    "mergeinfo log-revprops ) "
                    "46:svn://subversion.io/apache/subversion ) ";

  char kHeaderE1[] = "(( ) ";
  char kHeaderE3[] =
      "( 2 ( edit-pipeline svndiff1 absent-entries depth mergeinfo log-revprops"
      ") "
      "42:svn://git.oschina.net:1234/subversion/home "
      "53:SVN/1.8.13-SlikSvn-1.8.13-X64 (x64-microsoft-windows) ( ) )";

  char kHeader4[] =
      "( 2 ( edit-pipeline svndiff1 absent-entries depth mergeinfo log-revprops"
      ") "
      "73:svn://example@subversion.io/subversion/subversion/trunk/sources/"
      "libsvn_ra "
      "53:SVN/1.8.13-SlikSvn-1.8.13-X64 (x64-microsoft-windows) ( ) )";
  ParseS(kHeader, sizeof(kHeader) - 1);
  ParseS(kHeader2, sizeof(kHeader2) - 1);
  ParseS(kHeaderE, sizeof(kHeaderE) - 1);
  ParseS(kHeaderE1, sizeof(kHeaderE1) - 1);
  ParseS(kHeaderE3, sizeof(kHeaderE3) - 1);
  ParseS(kHeader4, sizeof(kHeader4) - 1);
  return 0;
}
