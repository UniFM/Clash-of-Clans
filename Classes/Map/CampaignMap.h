#pragma once
#ifndef __CAMPAIGN_MAP_H__
#define __CAMPAIGN_MAP_H__

#include "BaseMap.h"

class CampaignMap : public BaseMap
{
public:
    static CampaignMap* create(int levelId);
    virtual bool init(int levelId);

private:
    void setupLevel(int levelId);
};

#endif // __CAMPAIGN_MAP_H__
