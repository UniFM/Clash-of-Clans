#include "HomeVillageMap.h"
#include "cocos2d.h"

using namespace cocos2d;

HomeVillageMap* HomeVillageMap::sInstance = nullptr;

HomeVillageMap::HomeVillageMap()
    : _hasPendingBuilding(false)
    , _pendingBuildingType(BuildingType::TOWN_HALL)
{
}

HomeVillageMap::~HomeVillageMap()
{
    CCLOG("HomeVillageMap destructor called");
    if (sInstance == this) {
        sInstance = nullptr;
    }
}

HomeVillageMap* HomeVillageMap::getInstance(const std::string& mapImagePath) {
    if (sInstance) {
        if (sInstance->getReferenceCount() == 0) {
            sInstance = nullptr;
        }
    }
    
    if (!sInstance) {
        sInstance = new (std::nothrow) HomeVillageMap();
        if (sInstance && sInstance->init(mapImagePath)) {
            sInstance->autorelease();
            sInstance->retain();
        }
        else {
            CC_SAFE_DELETE(sInstance);
        }
    } else {
        auto parent = sInstance->getParent();
        if (parent) {
            parent->removeChild(sInstance, false);
        }
    }
    return sInstance;
}

HomeVillageMap* HomeVillageMap::create(const std::string& mapImagePath)
{
    HomeVillageMap* homeVillageMap = new (std::nothrow) HomeVillageMap();
    if (homeVillageMap && homeVillageMap->init(mapImagePath))
    {
        homeVillageMap->autorelease();
        return homeVillageMap;
    }
    CC_SAFE_DELETE(homeVillageMap);
    return nullptr;
}

void HomeVillageMap::destroyInstance() {
    if (sInstance) {
        auto parent = sInstance->getParent();
        if (parent) {
            parent->removeChild(sInstance, false);
        }
        sInstance->release();
        sInstance = nullptr;
    }
}

bool HomeVillageMap::init(const std::string& mapImagePath)
{
    // 设置特定于家乡村庄的地图参数
    _mapWidth = 3460.0f;
    _mapHeight = 2480.0f;
    _grassRectWidth = 2400.0f;
    _grassRectHeight = 1800.0f;
    _grassOffsetX = 530.0f;
    _grassOffsetY = 252.0f;
    _gridCols = 44;
    _gridRows = 44;

    if (!BaseMap::init(mapImagePath))
    {
        return false;
    }
        
    return true;
}

bool HomeVillageMap::onTouchBegan(Touch* touch, Event* event)
{
    return BaseMap::onTouchBegan(touch, event);
}

void HomeVillageMap::onTouchMoved(Touch* touch, Event* event)
{
    BaseMap::onTouchMoved(touch, event);
}

void HomeVillageMap::onTouchEnded(Touch* touch, Event* event)
{
    BaseMap::onTouchEnded(touch, event);
}

void HomeVillageMap::startBuildingPlacement(BuildingType buildingType)
{
    CCLOG("HomeVillageMap: Starting building placement for type: %d", (int)buildingType);
}
