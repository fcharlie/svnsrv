/*
* Daemon.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_DAEMON_H
#define SVNSRV_DAEMON_H

int CreateDaemon();
int RegisterSignalHandle(bool isDaemon);
void CrashHandle(const char *data, int size);
bool StoreDaemonPID(const std::string &pidFile);
bool StopDaemonService(const std::string &pidFile);
bool RestartDaemonService(const std::string &pidFile);

#endif
