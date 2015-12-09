/*
* config.h
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_CONFIG_H
#define SVNSRV_CONFIG_H
#define SVNSRV_MAJOR 0
#define SVNSRV_MINOR 0
#define SVNSRV_SUBOR 1
#define SVNSRV_FIXOR 0

#define TOSTRING2(x) #x
#define TOSTRING1(x) TOSTRING2(x)
#define TOSTRING(x) TOSTRING1(x)

#define CONFIG_VERSION                                                         \
  TOSTRING(SVNSRV_MAJOR.SVNSRV_MINOR.SVNSRV_SUBOR.SVNSRV_FIXOR) /// "0.0.1.0"

#endif