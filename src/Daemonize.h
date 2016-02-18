/*
* Daemonize.h
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_DAEMON_H
#define SVNSRV_DAEMON_H

int Daemonize(const std::string &pidfile);
bool DaemonWait(int Argc, char **Argv, bool crashRestart);
int DaemonSignalMethod();
int ForegroundSignalMethod();
bool DaemonStop(const std::string &pidFile);
bool DaemonRestart(const std::string &pidFile);

#endif
