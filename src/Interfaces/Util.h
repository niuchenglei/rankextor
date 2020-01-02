#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

using std::string;
using std::vector;

namespace rank {

extern uint64_t __crc64_table__[8][256];
static uint64_t crc64(const char* buffer) {
  uint64_t crc = 0;

  // slow VS fast version of CRC-64
  // see more: https://matt.sh/redis-crcspeed
  // The original Redis CRC-64 slow version using the Sarwate method:
  /*while(*buffer != '\0') {
      crc = crctab64[(crc ^ *(buffer++)) & 0xff] ^ (crc >> 8);
  }
  return crc;*/

  // */
  int len = strlen(buffer);
  while (len >= 8) {
    crc ^= *(uint64_t*)buffer;
    crc =
        __crc64_table__[7][crc & 0xff] ^ __crc64_table__[6][(crc >> 8) & 0xff] ^
        __crc64_table__[5][(crc >> 16) & 0xff] ^
        __crc64_table__[4][(crc >> 24) & 0xff] ^
        __crc64_table__[3][(crc >> 32) & 0xff] ^
        __crc64_table__[2][(crc >> 40) & 0xff] ^
        __crc64_table__[1][(crc >> 48) & 0xff] ^ __crc64_table__[0][crc >> 56];
    buffer += 8;
    len -= 8;
  }
  while (len) {
    crc = __crc64_table__[0][(crc ^ *buffer++) & 0xff] ^ (crc >> 8);
    len--;
  }
  return crc;
}

static bool isFloat(const std::string& str, float* f) {
  try {
    string str1(str);
    boost::trim(str1);
    *f = boost::lexical_cast<float>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not float string: str=%s", str.c_str());
    return false;
  }
  return true;
}

static bool isLong(const std::string& str, long* l) {
  try {
    string str1(str);
    boost::trim(str1);
    *l = boost::lexical_cast<long>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not long string: str=%s", str.c_str());
    return false;
  }
  return true;
}

static bool valid_number(const string& str) { 
    int i = 0, j = str.length() - 1; 
  
    // Handling whitespaces 
    while (i < str.length() && str[i] == ' ') 
        i++; 
    while (j >= 0 && str[j] == ' ') 
        j--; 
  
    if (i > j) 
        return false; 
  
    // if string is of length 1 and the only 
    // character is not a digit 
    if (i == j && !(str[i] >= '0' && str[i] <= '9')) 
        return false; 
  
    // If the 1st char is not '+', '-', '.' or digit 
    if (str[i] != '.' && str[i] != '+'
        && str[i] != '-' && !(str[i] >= '0' && str[i] <= '9')) 
        return false; 
  
    // To check if a '.' or 'e' is found in given 
    // string. We use this flag to make sure that 
    // either of them appear only once. 
    bool flagDotOrE = false; 
  
    for (i; i <= j; i++) { 
        // If any of the char does not belong to 
        // {digit, +, -, ., e} 
        if (str[i] != 'e' && str[i] != '.'
            && str[i] != '+' && str[i] != '-'
            && !(str[i] >= '0' && str[i] <= '9')) 
            return false; 
  
        if (str[i] == '.') { 
            // checks if the char 'e' has already 
            // occurred before '.' If yes, return 0. 
            if (flagDotOrE == true) 
                return false; 
  
            // If '.' is the last character. 
            if (i + 1 > str.length()) 
                return false; 
  
            // if '.' is not followed by a digit. 
            if (!(str[i + 1] >= '0' && str[i + 1] <= '9')) 
                return false; 
        } 
  
        else if (str[i] == 'e') { 
            // set flagDotOrE = 1 when e is encountered. 
            flagDotOrE = true; 
  
            // if there is no digit before 'e'. 
            if (!(str[i - 1] >= '0' && str[i - 1] <= '9')) 
                return false; 
  
            // If 'e' is the last Character 
            if (i + 1 > str.length()) 
                return false; 
  
            // if e is not followed either by 
            // '+', '-' or a digit 
            if (str[i + 1] != '+' && str[i + 1] != '-'
                && (str[i + 1] >= '0' && str[i] <= '9')) 
                return false; 
        } 
    } 
  
    /* If the string skips all above cases, then  
    it is numeric*/
    return true; 
} 

static bool isUint64(const std::string& str, uint64_t* l) {
  try {
    string str1(str);
    //boost::trim(str1);
    *l = boost::lexical_cast<uint64_t>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not long string: str=%s", str.c_str());
    return false;
  }
  return true;
}

static bool isInt64(const std::string& str, int64_t* l) {
  try {
    string str1(str);
    boost::trim(str1);
    *l = boost::lexical_cast<int64_t>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not long string: str=%s", str.c_str());
    return false;
  }
  return true;
}

static std::string getDirPath(std::string& full) {
  size_t len = full.length();
  int i;
  for (i = len - 1; i >= 0; i--) {
    if (full[i] == '/') break;
  }
  string ret = full.substr(0, i);
  return ret;
}

static std::string getFileName(std::string& full) {
  size_t len = full.length();
  int i, j = len;
  for (i = len - 1; i >= 0; i--) {
    if (full[i] == '.') j = i;
    if (full[i] == '/') break;
  }
  string ret = full.substr(i + 1, j - i - 1);
  return ret;
}

static struct timeval time_minus(struct timeval end, struct timeval start) {
  struct timeval delta;
  delta.tv_sec = end.tv_sec - start.tv_sec;
  delta.tv_usec = end.tv_usec - start.tv_usec;
  while (delta.tv_usec < 0) {
    if (delta.tv_sec > 0) {
      delta.tv_sec--;
      delta.tv_usec = 1000000 + delta.tv_usec;
    } else {
      // FATAL(LOGNAME, "time_minus error : end:%ld,%ld < start:%ld,%ld",
      // end.tv_sec, end.tv_usec, start.tv_sec,start.tv_usec);
      delta.tv_sec = 0;
      delta.tv_usec = 0;
      return delta;
    }
  }
  return delta;
}

static struct timeval time_add(struct timeval time1, struct timeval time2) {
  struct timeval sum_time;
  sum_time.tv_sec = time1.tv_sec + time2.tv_sec;
  sum_time.tv_usec = time1.tv_usec + time2.tv_usec;
  while (sum_time.tv_usec >= 1000000) {
    sum_time.tv_sec++;
    sum_time.tv_usec = sum_time.tv_usec - 1000000;
  }
  return sum_time;
}

static unsigned long hash_str(const std::string& str) {
  unsigned long __h = 0;
  for (size_t i = 0; i < str.size(); i++) __h = 5 * __h + str[i];
  return __h;
}
static void toStringFromLong(uint64_t id, char* buf) {
  static char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  char* p = buf;
  do {
    *p++ = digits[id % 10];
    id = id / 10;
  } while (id);
  *p = '\0';
  std::reverse(buf, p);
}

static string trim(string s) {
  int i = 0;
  if (s.length() == 0) return "";

  while (s[i] == ' ') {
    i++;
  }
  s = s.substr(i);
  i = s.size() - 1;
  if (i == -1) return "";

  while (s[i] == ' ') {
    i--;
  }
  s = s.substr(0, i + 1);
  return s;
}

static std::vector<std::string> split(const std::string& str, char delim) {
  size_t last = 0;
  size_t index = str.find_first_of(delim, last);

  int len;
  vector<string> ret;
  string tmp = "";
  while (index != std::string::npos) {
    tmp = trim(str.substr(last, index - last));
    ret.push_back(tmp);

    last = index + 1;
    index = str.find_first_of(delim, last);
  }
  if (index - last > 0) {
    tmp = trim(str.substr(last, index - last));
    ret.push_back(tmp);
  }

  return ret;
}

/*inline std::string toStringI(const int value)
{
   // The largest 32-bit integer is 4294967295, that is 10 chars
   // On the safe side, add 1 for sign, and 1 for trailing zero
   char buffer[32];
   sprintf(buffer, "%i", value);
   return std::string(buffer);
}
inline std::string toStringF(const float value)
{
   char buffer[32];
   sprintf(buffer, "%f", value);
   return std::string(buffer);
}
std::string cast(int i) {
    ostringstream os;
    os << i;
    return os.str();
}
*/

static std::string binary2hex(const std::string src) {
  size_t len = src.length();
  string v = src;

  // 如果长度不为8的整数倍，则补齐0
  if (len % 8 != 0) {
    for (int i = 0; i < 8 - len % 8; i++) v += "0";
  }

  return v;
}

static void Int64ToString(uint64_t candidate, char* buf) {
  char* decode = buf;
  if (0 == candidate) {
    *(decode) = '0';
    *(decode + 1) = '\0';
    return;
  }
  int i = 0;
  while (0 != candidate) {
    decode[i++] = candidate % 10 + '0';
    candidate = candidate / 10;
  }
  *(decode + i) = '\0';
  int j = i - 1;
  i = 0;
  while (i < j) {
    *(decode + i) ^= *(decode + j);
    *(decode + j) ^= *(decode + i);
    *(decode + i) ^= *(decode + j);
    i++;
    j--;
  }
}

static time_t ConvertStrtoTime(char* szTime) {
  tm tm_;
  time_t t_;
  strptime(szTime, "%Y-%m-%d %H:%M:%S", &tm_);  //将字符串转换为tm时间
  tm_.tm_isdst = -1;
  t_ = mktime(&tm_);  //将tm时间转换为秒时间
  return t_;
}

static int GetHourOfDay(){
  time_t theTime = time(NULL);
  struct tm *aTime = localtime(&theTime);
  return aTime->tm_hour;
}
}  // namespace rank

namespace boost {
template <>
inline int lexical_cast(const std::string& arg) {
  char* stop;
  int res = strtol(arg.c_str(), &stop, 10);
  if (*stop != 0)
    throw_exception(bad_lexical_cast(typeid(int), typeid(std::string)));
  return res;
}
/*template<>
inline int64_t lexical_cast(const std::string& arg) {
    char* stop;
    long int res = strtoll(arg.c_str(), &stop, 10);
    if ( *stop != 0 ) throw_exception(bad_lexical_cast(typeid(int64_t),
typeid(std::string)));
    return res;
}
template<>
inline uint64_t lexical_cast(const std::string& arg) {
    char* stop;
    uint64_t res = strtoull(arg.c_str(), &stop, 10);
    if ( *stop != 0 ) throw_exception(bad_lexical_cast(typeid(uint64_t),
typeid(std::string)));
    return res;
}*/
template <>
inline std::string lexical_cast(const int& arg) {
  char buffer[65];  // large enough for arg < 2^200
  sprintf(buffer, "%d", arg);
  return std::string(buffer);  // RVO will take place here
}
template <>
inline std::string lexical_cast(const int64_t& arg) {
  char buffer[65];  // large enough for arg < 2^200
  sprintf(buffer, "%" PRIu64 "", arg);
  return std::string(buffer);  // RVO will take place here
}
template <>
inline std::string lexical_cast(const uint64_t& arg) {
  char buffer[65];  // large enough for arg < 2^200
  sprintf(buffer, "%" PRId64 "", arg);
  return std::string(buffer);  // RVO will take place here
}
template <>
inline std::string lexical_cast(const float& arg) {
  char buffer[65];  // large enough for arg < 2^200
  sprintf(buffer, "%f", arg);
  return std::string(buffer);  // RVO will take place here
}
template <>
inline std::string lexical_cast(const double& arg) {
  char buffer[65];  // large enough for arg < 2^200
  sprintf(buffer, "%f", arg);
  return std::string(buffer);  // RVO will take place here
}

}  // namespace boost

namespace rank {

//#if __cplusplus < 201103L
#define TO_STRING(x) boost::lexical_cast<std::string>(x)
#define STRING_TO_INT(x) strtol(x.c_str(), NULL, 10)
#define STRING_TO_FLOAT(x) strtof(x.c_str(), NULL)
#define STRING_TO_INT64(x) strtoll(x.c_str(), NULL, 10)
#define STRING_TO_UINT64(x) strtoull(x.c_str(), NULL, 10)
/*#else
#define TO_STRING(x) std::to_string(x)
#define STRING_TO_INT(x) std::stoi(x)
#endif*/

}  // namespace rank

#endif
