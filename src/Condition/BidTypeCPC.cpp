#include "Condition/BidType.h"

using namespace boost;

namespace rank {

REGISTE_CLASS("cpc", BidTypeCPC)

bool BidTypeCPC::Check(const RankAdInfo& adinfo, RankDataContext& ptd,
                    const RankRequest* fisher_request) {
  if (adinfo.bid_type == rank::CPC)
    return true;
  return false;
}

}  // namespace rank
