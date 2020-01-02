#ifndef PAIR_READER_H
#define PAIR_READER_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/CommonType.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"

namespace rank {

class PairReader {
 public:
  PairReader(std::string config_file);
  bool isValid();
  template <typename T>
  bool getValue(std::string name, T& value);

 protected:
  bool mIsValid;
  boost::unordered_map<std::string, std::string> mPairs;
};

}  // namespace rank

#endif
