#include "LogServerRequestData.h"

#include <common/Utilities.h>

namespace sf1r
{

std::string UUID2DocidList::toString() const
{
    std::ostringstream oss;
    oss << Utilities::uint128ToUuid(uuid_) << " ->";

    for (size_t i = 0; i < docidList_.size(); i++)
    {
        oss << " " << docidList_[i];
    }

    return oss.str();
}

std::string Docid2UUID::toString() const
{
    std::ostringstream oss;
    oss << docid_ << " ->" << Utilities::uint128ToUuid(uuid_);

    return oss.str();
}

}
