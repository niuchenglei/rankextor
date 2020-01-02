#include "Condition/BidType.h"

using namespace boost;

namespace rank {

REGISTE_CLASS("cpm", BidTypeCPM)

bool BidTypeCPM::Check(const RankAdInfo& adinfo, RankDataContext& ptd,
                    const RankRequest* fisher_request) {
  if (adinfo.bid_type == rank::CPM)
    return true;
  return false;
}

}  // namespace rank
