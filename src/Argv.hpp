/*
* Argv.hpp
* Miracle Argv Copy
* author: Force.Charlie
* Date: 2016.01
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef MIRACLE_ARGV_HPP
#define MIRACLE_ARGV_HPP
#include <vector>
#include <cstdlib>
#include <cstring>

class SavedArgv {
private:
  std::vector<char *> Argvs;

public:
  SavedArgv &operator=(const SavedArgv &) = delete;
  SavedArgv(SavedArgv &) = delete;
  SavedArgv(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
      Argvs.push_back(strdup(argv[i]));
    }
  }
  ~SavedArgv() {
    ////
    for (auto &a : Argvs) {
      std::free(a);
    }
  }
  char **Argv() const { return const_cast<decltype(Argv())>(Argvs.data()); }
  int Argc() const { return Argvs.size(); }
};

#endif
