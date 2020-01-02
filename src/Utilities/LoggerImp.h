#ifndef LOGGER_C_IMP_H
#define LOGGER_C_IMP_H

#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <string>
#include <sstream>

namespace rank {

#define LOG_DEBUG(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->debug("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_INFO(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->info("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_NOTICE(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->notice("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_WARN(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->warn("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_ERROR(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->error("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_FATAL(category_name, log_format, ...) \
  { LoggerImp::getInstance()->rootCategory->fatal("%s:%d(%s) "log_format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }


//enum myPriority{EMERG,FATAL,ALERT,CRIT,ERROR,WARN,NOTICE,INFO,DEBUG};

class LoggerImp {
public:
  static LoggerImp* getInstance();  
  static void destroy();
  log4cpp::Category* rootCategory;

private:
  static LoggerImp* _plog;

  LoggerImp();
  ~LoggerImp();
};

}

#endif
