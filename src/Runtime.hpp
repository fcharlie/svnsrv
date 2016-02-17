/*
* Runtime.hpp
* OS Abstract Function
* author: Force.Charlie
* Date: 2016.02
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_RUNTIME_HPP
#define SVNSRV_RUNTIME_HPP

// bool GetMainExecuteFileDir
#include <string>

bool GetProcessImageFileFolder(std::string &dir);
bool PathCombineHomeExists(std::string path, const char *relativePath);
bool PathFileIsExists(const std::string &path);

#endif
