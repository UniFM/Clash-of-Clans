#pragma once
#ifndef __RESOURCE_BUILDING_H__
#define __RESOURCE_BUILDING_H__

#include "Building.h"

class ResourceBuilding : public Building
{
public:
    static ResourceBuilding* create(BuildingType type, int level = 1);
    virtual bool init(BuildingType type, int level);

    // 资源生成
    void startGenerating();
    void stopGenerating();
    int getGenerationRate() const { return _generationRate; }

    // 更新（每帧调用）
    void update(float dt) override;

protected:
    float _generationInterval;  // 生成间隔（秒）
    float _accumulatedTime;     // 累积时间
    int _generationRate;        // 每次生成的资源量

    void updateGenerationRate();
};

#endif // __RESOURCE_BUILDING_H__

