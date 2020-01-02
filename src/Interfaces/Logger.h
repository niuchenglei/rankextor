/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "Interfaces/Version.h"
#include "Interfaces/Config.h"

//#define PRINT_LOG

namespace rank {

static const std::string LOGNAME = "log";

#ifdef PRINT_LOG
#define DEBUG(category_name, log_format, ...) \
  { fprintf(stderr, "[DEBUG] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define INFO(category_name, log_format, ...) \
  { fprintf(stderr, "[INFO] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define NOTICE(category_name, log_format, ...) \
  { fprintf(stderr, "[NOTICE] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define WARN(category_name, log_format, ...) \
  { fprintf(stderr, "[WARN] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define ERROR(category_name, log_format, ...) \
  { fprintf(stderr, "[ERROR] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#define FATAL(category_name, log_format, ...) \
  { fprintf(stderr, "[FATAL] "); fprintf(stderr, log_format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#endif

#ifdef NO_LOG 
#define DEBUG(category_name, log_format, ...) {} 
#define INFO(category_name, log_format, ...) {}
#define NOTICE(category_name, log_format, ...) {}
#define WARN(category_name, log_format, ...) {}
#define ERROR(category_name, log_format, ...) {}
#define FATAL(category_name, log_format, ...) {}
#endif

}  // namespace rank

#endif
