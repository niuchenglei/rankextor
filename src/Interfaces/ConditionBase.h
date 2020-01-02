#ifndef CONDITION_BASE_H
#define CONDITION_BASE_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/RankDataContext.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"

namespace rank {

class ConditionBase : public AutoUpdate {
 public:
  virtual bool Initialize() = 0;
  virtual bool Check(const RankAdInfo& adinfo, RankDataContext& ptd,
                     const RankRequest* fisher_request = NULL) = 0;
  virtual const std::string& name() = 0;
};

}  // namespace rank

#endif
