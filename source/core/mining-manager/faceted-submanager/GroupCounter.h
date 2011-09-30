///
/// @file GroupCounter.h
/// @brief base class to count docs for property values
/// @author Jun Jiang <jun.jiang@izenesoft.com>
/// @date Created 2011-09-05
///

#ifndef SF1R_GROUP_COUNTER_H
#define SF1R_GROUP_COUNTER_H

#include "faceted_types.h"

NS_FACETED_BEGIN

class GroupRep;

class GroupCounter
{
public:
    GroupCounter() {}

    virtual ~GroupCounter() {}

    virtual void addDoc(docid_t doc) = 0;

    virtual void getGroupRep(GroupRep &groupRep) = 0;
};

NS_FACETED_END

#endif 
