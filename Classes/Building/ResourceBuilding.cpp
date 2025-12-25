#include "ResourceBuilding.h"
#include "ResourceManager.h"

ResourceBuilding* ResourceBuilding::create(BuildingType type, int level)
{
    ResourceBuilding* building = new (std::nothrow) ResourceBuilding();
    if (building && building->init(type, level))
    {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool ResourceBuilding::init(BuildingType type, int level)
{
    if (!Building::init(type, level))
        return false;

    _generationInterval = 1.0f; // 每秒生成一次
    _accumulatedTime = 0.0f;
    updateGenerationRate();

    this->scheduleUpdate();
    return true;
}

void ResourceBuilding::startGenerating()
{
    this->scheduleUpdate();
}

void ResourceBuilding::stopGenerating()
{
    this->unscheduleUpdate();
}

void ResourceBuilding::update(float dt)
{
    if (isDestroyed())
        return;

    _accumulatedTime += dt;
    if (_accumulatedTime >= _generationInterval)
    {
        _accumulatedTime = 0.0f;

        auto resourceMgr = ResourceManager::getInstance();
        if (_type == BuildingType::GOLD_MINE)
        {
            resourceMgr->addGold(_generationRate);
        }
        else if (_type == BuildingType::ELIXIR_COLLECTOR)
        {
            resourceMgr->addElixir(_generationRate);
        }
    }
}

void ResourceBuilding::updateGenerationRate()
{
    // 根据等级设置生成速率
    _generationRate = 10 + (_level - 1) * 5;
}

