#ifndef MTAI_H
#define MTAI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPORT extern "C" 

EXPORT int mtai_init(const char* config_file, int worker_num);
EXPORT int mtai_uninit();

EXPORT char* mtai_predict(const char* request, int bytes_request, int worker_idx, int* bytes_write);

EXPORT void mtai_free(char* ptr);

EXPORT int mtai_test();
EXPORT char* mtai_test_pb(const char* request, int bytes_request, int worker_idx, int* bytes_write);
EXPORT int mtai_test_broken();
EXPORT int mtai_test_oom();
EXPORT int mtai_test_malloc();

#endif
