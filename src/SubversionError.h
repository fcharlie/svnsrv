/*
* SubversionError.h
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/

#ifndef SUBVERSION_ERROR_H
#define SUBVERSION_ERROR_H

//( failure ( 21:incorrect credentials ) )
typedef struct ErrorMessageStruct {
  int length;
  const char *message;
} ErrorMessage;
#define SVN_ERR_OFFSET 0
#define SVN_ERR_UNKNOWN SVN_ERR_OFFSET
#define SVN_INTERNAL_SERVER_ERROR SVN_ERR_OFFSET + 1
#define SVN_ERR_BAD_URL SVN_ERR_OFFSET + 2
#define SVN_ERR_CONNECT_FAILE SVN_ERR_OFFSET + 3

#define ERROR_PUSH(str)                                                        \
  { sizeof(str), str }

#endif
