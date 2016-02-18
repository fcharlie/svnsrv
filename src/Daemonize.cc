/*
* Daemonize.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>
#include "klog.h"
#include "Daemonize.h"
#if defined(_WIN32)
#include "daemonize_win.inl"
#elif defined(__linux__)
#include "daemonize_linux.inl"
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(__NetBSD__) ||     \
    defined(__DragonFly__)
#include "daemonize_bsd.inl"
#else
#error "Not implemented on this platform compatibility"
#endif
