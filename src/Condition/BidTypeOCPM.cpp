#include "Condition/BidType.h"

using namespace boost;

namespace rank {

REGISTE_CLASS("ocpm", BidTypeOCPM)

bool BidTypeOCPM::Check(const RankAdInfo& adinfo, RankDataContext& ptd,
                    const RankRequest* fisher_request) {
  if (adinfo.bid_type == rank::OCPM)
    return true;
  return false;
}

}  // namespace rank
