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

//////// getopt replace
#if defined(_MSC_VER)
#define REPLACE_GETOPT
#define REPLACE_GETOPT_LONG
#endif
#if defined(_MSC_VER) || defined(__NetBSD__)
#define REPLACE_GETOPT_LONG_ONLY
#endif

#if defined(REPLACE_GETOPT)
// from getopt.h
#define no_argument 0
#define required_argument 1
#define optional_argument 2

// option structure
struct option {
  const char *name;
  // has_arg can't be an enum because some compilers complain about
  // type mismatches in all the code that assumes it is an int.
  int has_arg;
  int *flag;
  int val;
};

int getopt(int argc, char *const argv[], const char *optstring);

// from getopt.h
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

// defined in unistd.h
extern int optreset;
#else
#include <unistd.h>
#include <getopt.h>
#endif

#if defined(REPLACE_GETOPT_LONG)
int getopt_long(int argc, char *const *argv, const char *optstring,
                const struct option *longopts, int *longindex);
#endif

#if defined(REPLACE_GETOPT_LONG_ONLY)
int getopt_long_only(int argc, char *const *argv, const char *optstring,
                     const struct option *longopts, int *longindex);
#endif

/*
* SavedArgv RAII based Argv class
* when some one change process title, argv has been changed
* but app require parse argv
*/
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
    for (auto &a : Argvs) {
      std::free(a);
    }
  }
  char **Argv() const { return const_cast<decltype(Argv())>(Argvs.data()); }
  int Argc() const { return Argvs.size(); }
};

#endif
