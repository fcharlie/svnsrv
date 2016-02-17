/*
* TOMLParse.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef TOML_PARSE_HPP
#define TOML_PARSE_HPP

bool ParseServiceProfile(const char *configfile, NetworkServerArgs &na,
                         LauncherArgs &la);
// bool InitializeRouterTable(const std::string &routerFile);
#endif
