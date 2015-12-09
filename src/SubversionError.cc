/*
* SubversionError.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include "SubversionError.h"

static ErrorMessageStruct __errorMap[] = {
    ERROR_PUSH("unknown"), ERROR_PUSH("internal server error"),
    ERROR_PUSH("Bogus URL"), ERROR_PUSH("cannot connect backend server")};

const ErrorMessage &getErrorMessage(int offset) {
  static auto n = sizeof(__errorMap) / sizeof(__errorMap[0]);
  if (offset <= n - 1) {
    return __errorMap[offset];
  }
  return __errorMap[0];
}

#ifdef _UTEST
int main() {
  auto e = getErrorMessage(2);
  printf("Error Message-> %d:%s\n", e.length, e.message);
}
#endif
