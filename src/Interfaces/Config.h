/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef APPCONFIG_H
#define APPCONFIG_H

#if !defined(__GNUC__) || __GNUC__ != 4
//#error "Requires version 4 of the GNU g++ compiler"
#endif

#define __linux__
#if !defined(__WIN32) && !defined(__linux__)
#error Unrecognized build platform
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdexcept>

#define NEST_NAMESPACE "ranking_plugin"

#ifndef NULL
#define NULL 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 8192
#endif
#define UNIX_API
#define EXPORT_SYMBOL

#if defined(__linux__)  // defined(__WIN32)
#define LINUX
#else
#define OTHER_OS
#endif

#define PTR_SIZE __SIZEOF_POINTER__
#define LONG_SIZE __SIZEOF_LONG__

#define SLASH std::string("/")
#define SLASH_T "/"
#define SLASH_C '/'

// allow user to use __stdcall on unix and do the correct thing
//#define __stdcall

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define PI 3.14159265358979323e0  // PI

#define MAX_THREADS 8
extern int OMP_THREAD;
extern int NUM_THREAD;

// 4,294,967,295 2^32-1
#define FEATURE_SCALE 1000000

#define BID_CPM rank::CPM
#define BID_CPV rank::CPV
#define BID_OCPM rank::OCPM
#define BID_CPE rank::CPE
#define BID_CPC rank::CPC

#endif
