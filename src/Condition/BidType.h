#ifndef BID_TYPE_COND_H
#define BID_TYPE_COND_H

#include <sstream>
#include <time.h>
//#include <sharelib/util/string_util.h>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/ConditionBase.h"

namespace rank {

class BidTypeCPM : public ConditionBase {
 public:
  OBJECT_CLASS(BidTypeCPM)

  BidTypeCPM() {}
  ~BidTypeCPM() {}
  bool Initialize() { return true; }
  bool Check(const RankAdInfo& adinfo, RankDataContext& ptd,
             const RankRequest* fisher_request = NULL);
  bool Update() { return false; }
  const std::string& name() { return getRegistName(); }
};


class BidTypeOCPM : public ConditionBase {
 public:
  OBJECT_CLASS(BidTypeOCPM)

  BidTypeOCPM() {}
  ~BidTypeOCPM() {}
  bool Initialize() { return true; }
  bool Check(const RankAdInfo& adinfo, RankDataContext& ptd,
             const RankRequest* fisher_request = NULL);
  bool Update() { return false; }
  const std::string& name() { return getRegistName(); }
};


class BidTypeCPC : public ConditionBase {
 public:
  OBJECT_CLASS(BidTypeCPC)

  BidTypeCPC() {}
  ~BidTypeCPC() {}
  bool Initialize() { return true; }
  bool Check(const RankAdInfo& adinfo, RankDataContext& ptd,
             const RankRequest* fisher_request = NULL);
  bool Update() { return false; }
  const std::string& name() { return getRegistName(); }
};

}  // namespace rank

#endif
