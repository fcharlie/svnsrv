/*
* Daemonize.h
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_DAEMON_H
#define SVNSRV_DAEMON_H

int Daemonize(const std::string &pidfile);
int DaemonSignalMethod();
int SignalINTActive();
bool DaemonStop(const std::string &pidFile);
bool DaemonRestart(const std::string &pidFile);

#endif
