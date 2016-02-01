/*
* ProcessTitle.hpp
* Miracle Rename Process Title Utility Function
* author: Force.Charlie
* Date: 2016.01
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef MIRACLE_PROCESS_TITLE_HPP
#define MIRACLE_PROCESS_TITLE_HPP
#include <stdlib.h>
#include <string.h>

/*
* Warning ,must include once
*/

static char *lastArgv;

/*
* relayout environ
*/
bool initializeProcessEnv(int argc, char **argv) {
  extern char **environ;
  char *p;
  size_t size = 0;
  int i = 0;
  for (; environ[i]; i++) {
    size += strlen(environ[i]) + 1;
  }
  p = (char *)malloc(size);
  if (p == nullptr) {
    return false;
  }

  lastArgv = argv[0];

  for (i = 0; argv[i]; i++) {
    if (lastArgv == argv[i]) {
      lastArgv = argv[i] + strlen(argv[i]) + 1;
    }
  }
  for (i = 0; environ[i]; i++) {
    if (lastArgv == environ[i]) {
      size = strlen(environ[i]) + 1;
      lastArgv = environ[i] + size;
      memcpy(p, environ[i], size);
      environ[i] = p;
      p = environ[i] + size;
    }
  }
  lastArgv--;
  return true;
}

/*
* change process title
*/
void changeProcessTitle(int argc, char **argv, const char *title) {
  char *p;
  argv[1] = NULL;
  strncpy(argv[0], title, lastArgv - argv[0]);
  p = argv[0] + strlen(argv[0]) + 1;
  if (lastArgv - p) {
    memset(p, 0, lastArgv - p);
  }
}

#endif
