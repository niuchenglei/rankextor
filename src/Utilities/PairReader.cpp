#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Interfaces/PairReader.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Logger.h"

namespace rank {

PairReader::PairReader(string config_file) {
  ifstream in(config_file.c_str());
  if (!in) {
    ERROR(LOGNAME, "can't find file=%s", config_file.c_str());
    return;
  }
  DEBUG(LOGNAME, "parsing file %s", config_file.c_str());

  string line, name, value;
  while (getline(in, line)) {
    vector<string> tmp;
    AdUtil::Split3(tmp, line, '=');
    if (tmp.size() != 2) {
      continue;
    }

    name = tmp[0];
    value = tmp[1];
    if (name[0] == '#' || name[0] == ' ') continue;
    name = AdUtil::Trim(name);
    value = AdUtil::Trim(value);
    if (name == "" || value == "") continue;

    mPairs.insert(make_pair(name, value));
  }
  in.close();
  DEBUG(LOGNAME, "parsing file %s success", config_file.c_str());
  mIsValid = true;
}

bool PairReader::isValid() { return mIsValid; }

template <>
bool PairReader::getValue(std::string name, int& value) {
  if (mPairs.find(name) == mPairs.end()) {
    DEBUG(LOGNAME, "can't find name:%s in config file", name.c_str());
    return false;
  }

  long vv;
  if (!isLong(mPairs[name], &vv)) {
    DEBUG(LOGNAME, "try case %s to int error", mPairs[name].c_str());
    return false;
  }

  value = vv;

  return true;
}

template <>
bool PairReader::getValue(std::string name, float& value) {
  if (mPairs.find(name) == mPairs.end()) {
    DEBUG(LOGNAME, "can't find name:%s in config file", name.c_str());
    return false;
  }

  float vv;
  if (!isFloat(mPairs[name], &vv)) {
    DEBUG(LOGNAME, "try case %s to float error", mPairs[name].c_str());
    return false;
  }

  value = vv;

  return true;
}

template <>
bool PairReader::getValue(std::string name, string& value) {
  if (mPairs.find(name) == mPairs.end()) {
    DEBUG(LOGNAME, "can't find name:%s in config file", name.c_str());
    return false;
  }

  value = mPairs[name];
  return true;
}

template <>
bool PairReader::getValue(std::string name, long& value) {
  if (mPairs.find(name) == mPairs.end()) {
    DEBUG(LOGNAME, "can't find name:%s in config file", name.c_str());
    return false;
  }

  long vv;
  if (!isLong(mPairs[name], &vv)) {
    DEBUG(LOGNAME, "try case %s to long error", mPairs[name].c_str());
    return false;
  }

  value = vv;

  return true;
}

template <>
bool PairReader::getValue(std::string name, double& value) {
  if (mPairs.find(name) == mPairs.end()) {
    DEBUG(LOGNAME, "can't find name:%s in config file", name.c_str());
    return false;
  }

  float vv;
  if (!isFloat(mPairs[name], &vv)) {
    DEBUG(LOGNAME, "try case %s to float error", mPairs[name].c_str());
    return false;
  }

  value = vv;

  return true;
}

}  // namespace rank
