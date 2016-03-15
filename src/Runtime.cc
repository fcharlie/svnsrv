/*
* Runtime.hpp
* OS Abstract Function
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include "Runtime.hpp"
#include <cstdint>

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
// Shlwapi.lib
// Shlwapi.dll
#include <Shlobj.h>
// Shell32.lib
// Shell32.dll
// char convert
class CharacterA {
private:
  char *raw_ = nullptr;

public:
  CharacterA(const wchar_t *wstr) {
    if (wstr == nullptr)
      return;
    int iTextLen =
        WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    raw_ = new char[iTextLen + 1];
    if (raw_ == nullptr)
      return;
    raw_[iTextLen] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, raw_, iTextLen, NULL, NULL);
  }
  ~CharacterA() {
    if (raw_) {
      delete[] raw_;
    }
  }
  const char *Get() {
    if (!raw_)
      return nullptr;
    return const_cast<const char *>(raw_);
  }
};

class CharacterW {
private:
  wchar_t *wstr = nullptr;

public:
  CharacterW(const char *str) {
    if (str == nullptr)
      return;
    // to wide char
    int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (unicodeLen == 0)
      return;
    wstr = new wchar_t[unicodeLen + 1];
    if (wstr == nullptr)
      return;
    wstr[unicodeLen] = 0;
    ::MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)wstr, unicodeLen);
  }
  ~CharacterW() {
    if (wstr)
      delete[] wstr;
  }
  const wchar_t *Get() {
    if (wstr == nullptr)
      return nullptr;
    return const_cast<const wchar_t *>(wstr);
  }
};

//
bool GetProcessImageFileFolder(std::string &dir) {
  ///
  wchar_t buffer[MAX_PATH] = {0};
  if (!GetModuleFileNameW(nullptr, buffer, MAX_PATH)) {
    return false;
  }
  if (!PathRemoveFileSpecW(buffer)) {
    return false;
  }
  CharacterA ca(buffer);
  dir = ca.Get();
  return true;
}

bool PathFileIsExists(const std::string &path) {
  if (path.empty())
    return false;
  CharacterW wc(path.c_str());
  return GetFileAttributesW(wc.Get()) != INVALID_FILE_ATTRIBUTES;
}

// SHGetKnownFolderPath:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb762188.aspx
// KNOWNFOLDERID
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd378457.aspx

bool PathCombineHomeExists(std::string &path, const char *relativePath) {
  if (relativePath == nullptr)
    return false;
#ifdef _MSC_VER
  wchar_t *pszBuffer = nullptr;
  //USERPROFILE
  if (SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &pszBuffer) != S_OK) {
    perror("SHGetKnownFolderPath cannot resolve user home !");
    return false;
  }
  CharacterA ca(pszBuffer);
  path = ca.Get();
  CoTaskMemFree(pszBuffer);
#else
  path=getenv("USERPROFILE");
#endif
  if (path.back() != '\\') {
    path.push_back('\\');
  }
  if (*relativePath == '\\' || *relativePath == '/') {
    relativePath++;
  }
  path.append(relativePath);
  return true;
}

#else
//// unix
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#if defined(__GNU__) && !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

static bool PathRemoveFileSpec(char *path, size_t len) {
  if (path == nullptr || len == 0)
    return false;
  auto p = path;
  auto end = p + strlen(path);
  while (p < end--) {
    if (*end == '/') {
      auto l = end - p;
      auto i = (l == 0 ? 1 : l);
      path[i] = 0;
      return true;
    }
  }
  return false;
}

/// Get ExecuteFile Folder

bool GetProcessImageFileFolder(std::string &dir) {
  char exe_path[PATH_MAX] = {0};
#if defined(__APPLE__)
  uint32_t size = sizeof(exe_path);
  if (_NSGetExecutablePath(exe_path, &size) == 0) {
    char link_path[MAXPATHLEN];
    if (realpath(exe_path, link_path)) {
      strncpy(exe_path, link_path, PATH_MAX);
    }
  } else {
    return false;
  }
#elif defined(__FreeBSD__)
  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;
  size_t cb = sizeof(exe_path);
  sysctl(mib, 4, exe_path, &cb, NULL, 0);
#elif defined(__NetBSD__)
  ssize_t len = readlink("/proc/curproc/exe", exe_path, PATH_MAX);
  if (len > 0) {
    exe_path[len] = 0;
  } else {
    return false;
  }
#elif defined(__DragonFly__)
  ssize_t len = readlink("/proc/curproc/file", exe_path, PATH_MAX);
  if (len > 0) {
    exe_path[len] = 0;
  } else {
    return false;
  }
#elif defined(__linux__)
  ssize_t len = readlink("/proc/self/exe", exe_path, PATH_MAX);
  if (len > 0) {
    exe_path[len] = 0;
  } else {
    return false;
  }
#else
#error GetProcessImageFileFolder is not implemented on this host yet.
#endif
  // remove file self
  if (!PathRemoveFileSpec(exe_path, PATH_MAX)) {
    return false;
  }
  dir = exe_path;
  return true;
}
///
bool PathFileIsExists(const std::string &path) {
  if (path.empty())
    return false;
  struct stat sb;
  if (stat(path.c_str(), &sb) != 0)
    return false;
  return true;
}
bool PathCombineHomeExists(std::string path, const char *relativePath) {
  if (relativePath == nullptr)
    return false;
  // char buffer[PATH_MAX] = {0};
  struct passwd *pw = getpwuid(getuid());
  if (pw == nullptr)
    return false;
  path = pw->pw_dir;
  if (path.back() != '/')
    path.push_back('/');
  if (*relativePath == '/')
    relativePath++;
  path.append(relativePath);
  if (!PathFileIsExists(path))
    return false;
  return true;
}
#endif
