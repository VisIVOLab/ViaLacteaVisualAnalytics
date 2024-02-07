#ifndef WCSTOOLS_WINDOWS_FIX_H
#define WCSTOOLS_WINDOWS_FIX_H

// Defined in types.h
#define off_t	_off_t
#define stat	_stat
#define S_IFDIR	_S_IFDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == S_IFDIR)

// Defined in string.h
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define ftruncate _chsize

// File descriptor associated with stdin, stdout, stderr
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

// Test for read permission
#define R_OK			4

// Defined in fcntl.h
#define O_RDONLY     _O_RDONLY
#define O_WRONLY     _O_WRONLY
#define O_RDWR       _O_RDWR
#define O_APPEND     _O_APPEND
#define O_CREAT      _O_CREAT
#define O_TRUNC      _O_TRUNC
#define O_EXCL       _O_EXCL
#define O_TEXT       _O_TEXT
#define O_BINARY     _O_BINARY
#define O_RAW        _O_BINARY
#define O_TEMPORARY  _O_TEMPORARY
#define O_NOINHERIT  _O_NOINHERIT
#define O_SEQUENTIAL _O_SEQUENTIAL
#define O_RANDOM     _O_RANDOM

// For webread.c
#include <winsock2.h>

// Replacement for sys/time.h
#include "win_time.h"

#endif // WCSTOOLS_WINDOWS_FIX_H