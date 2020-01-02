#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <syslog.h>

#include "Interfaces/Config.h"
#include "Interfaces/Version.h"
#include "Interfaces/Logger.h"
#include "Utilities/ConfigurationSettingsImp.h"

using std::string;
using std::vector;

namespace rank {
ConfigurationSettingsImp* ConfigurationSettingsImp::spInstance = NULL;
bool ConfigurationSettingsImp::mDestroyed = false;

static char config_file_path[4096];
char* get_config_file() { return config_file_path; }
/*
static char path[1024];
static const char* get_exe_path(){
#ifdef LINUX
    readlink("/proc/self/exe", path, 1024);
    int len = strlen(path);
    while(true){
        if(path[len-1] != '/')
            path[len-1] = '\0';
        else
            break;
        len--;
    }
    len = strlen(path);
    if(path[len-1] == '\0')
        path[len-1] = '\0';
#else
    getcwd(path, 1024);
#endif
    //printf("The current directory is: %s", path);
    return path;
}*/

ConfigurationSettingsImp* ConfigurationSettingsImp::instance() {
  if (spInstance == NULL) {
    spInstance = new ConfigurationSettingsImp;
  }

  return spInstance;
}

void ConfigurationSettingsImp::destroy() {
  if (mDestroyed) {
    return; throw std::logic_error(
        "Attempting to destroy ConfigurationSettings after destroying it.");
  }
  delete spInstance;
  spInstance = NULL;
  mDestroyed = true;
}

ConfigurationSettingsImp::ConfigurationSettingsImp()
    : mIsInitialized(false), mpReader(NULL) {

  // Reset Any Error Codes and mark as initialized
  mIsInitialized = false;

  // mHomePath = get_exe_path();

  mIsInitialized = deserialize(config_file_path);  // config_file);
}

ConfigurationSettingsImp::~ConfigurationSettingsImp() {
  if (mpReader) {
    delete mpReader;
    mpReader = NULL;
  }
}

bool ConfigurationSettingsImp::isInitialized() const { return mIsInitialized; }

static void split(const std::string& s, std::string& delim,
                  std::vector<std::string>* ret) {
  size_t last = 0;
  size_t index = s.find_first_of(delim, last);
  ret->clear();

  while (index != std::string::npos) {
    ret->push_back(s.substr(last, index - last));
    last = index + 1;
    index = s.find_first_of(delim, last);
  }
  if (index - last > 0) {
    ret->push_back(s.substr(last, index - last));
  }
}
string ConfigurationSettingsImp::getSetting(const string& key) const {
  string s = "/";
  vector<string> v;
  split(key, s, &v);
  if (v.size() != 2) return "";

  string value = mpReader->Get(v[0], v[1], "");
  // value = replaceGlobal(value);
  return value;
}

bool ConfigurationSettingsImp::deserialize(const string& fileName) {
  if (!mpReader) mpReader = new INIReader(fileName);

  if (mpReader->ParseError() < 0) {
    fprintf(stderr, "Error while parsing configuration file. [%s]",
            fileName.c_str());
    throw std::logic_error("Error while parsing configuration file.");
    return false;
  }

  return true;
}
}  // namespace rank
