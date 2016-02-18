/*
* Runtime.hpp
* OS Abstract Function
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_RUNTIME_HPP
#define SVNSRV_RUNTIME_HPP

#ifdef _WIN32
// windows
#define WIN32_LEAN_AND_MEAN
#include <sdkddkver.h>
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

/// NOTICE Mingw64 aslo have Processthreadsapi.h
#define _WIN32_WINNT _WIN32_WINNT_WINBLUE
#include <windows.h>
#include <Processthreadsapi.h>

#if defined(_MSC_VER) &&_MSC_VER<=1800
#define snprintf _snprintf
#endif

#else
// posix
#include <unistd.h>
#include <pthread.h>
#define GetCurrentProcessId getpid
#define GetCurrentThreadId pthread_self

#endif

#include <string>

bool GetProcessImageFileFolder(std::string &dir);
bool PathCombineHomeExists(std::string path, const char *relativePath);
bool PathFileIsExists(const std::string &path);

#endif
