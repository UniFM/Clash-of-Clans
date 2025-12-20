/*************************************************************
* @file     : Building.cpp
* @function ：建筑基类实现
* @author   : 叶芷含
* @note     ：最简单的建筑放置逻辑实现
**************************************************************/

#include "Building.h"
#include "Map/SceneMap.h"
#include "Map/HomeVillageMap.h"

USING_NS_CC;

Building::Building()
    : type(BuildingType::TOWN_HALL)
    , level(1)
    , gridSize(Size(2, 2))
    , gridPosition(Vec2::ZERO)
    , isPlaced(false)
    , inPlaceMode(false)
    , buildingSprite(nullptr)
    , shadowSprite(nullptr)
    , ownerMap(nullptr)
{
}

Building::~Building()
{
    // 清理资源
}

Building* Building::create(BuildingType type, int level)
{
    Building* building = new (std::nothrow) Building();
    if (building && building->init(type, level)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool Building::init(BuildingType type, int level)
{
    if (!Node::init()) {
        return false;
    }
    
    type = type;
    level = level;
    
    // 从配置获取建筑数据
    const BuildingData* buildingData = BuildingConfig::getBuildingData(type);
    if (!buildingData) {
        CCLOG("Error: Failed to get building data for type %d", static_cast<int>(type));
        return false;
    }
    
    gridSize = Size(buildingData->gridWidth, buildingData->gridHeight);
    
    // 获取等级数据
    const BuildingLevelStats* stats = BuildingConfig::getStats(type, level);
    if (!stats) {
        CCLOG("Error: Failed to get stats for building type %d level %d", static_cast<int>(type), level);
        return false;
    }
    
    // 创建建筑精灵
    buildingSprite = Sprite::create(stats->spriteName);
    if (buildingSprite) {
        this->addChild(buildingSprite);
        buildingSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    }
    
    // 创建阴影精灵（用于放置预览）
    shadowSprite = Sprite::create(stats->spriteName);
    if (shadowSprite) {
        this->addChild(shadowSprite);
        shadowSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        shadowSprite->setOpacity(128); // 半透明
        shadowSprite->setVisible(false);
    }
    
    updateAppearance();
    
    return true;
}

// 核心方法1：检查是否可以放置
bool Building::canPlaceAt(SceneMap* map, const Vec2& position)
{
    if (!map) return false;
    
    // 将世界坐标转换为网格坐标
    Vec2 gridPos = map->Cocos2dToTMX(position);
    
    // 检查是否在地图边界内，并且没有碰撞
    Size buildingSize = getBuildingSize();
    bool canPlace = map->canPlaceBuilding(position, buildingSize);
    
    CCLOG("Building placement check: pos(%.1f,%.1f) size(%.1fx%.1f) result:%s", 
          position.x, position.y, buildingSize.width, buildingSize.height,
          canPlace ? "YES" : "NO");
    
    return canPlace;
}

// 核心方法2：设置放置模式
void Building::setPlaceMode(bool placing)
{
    inPlaceMode = placing;
    
    if (placing) {
        // 进入放置模式
        buildingSprite->setVisible(false);
        shadowSprite->setVisible(true);
        CCLOG("Building entered place mode");
    } else {
        // 退出放置模式
        buildingSprite->setVisible(true);
        shadowSprite->setVisible(false);
        CCLOG("Building exited place mode");
    }
}

// 核心方法3：确认放置到地图
void Building::placeOnMap(SceneMap* map, const Vec2& position)
{
    if (!map || !canPlaceAt(map, position)) {
        CCLOG("Cannot place building at position (%.1f, %.1f)", position.x, position.y);
        return;
    }
    
    ownerMap = map;
    isPlaced = true;
    inPlaceMode = false;
    
    // 设置建筑位置
    this->setPosition(position);
    gridPosition = map->Cocos2dToTMX(position);
    
    // 恢复正常显示
    buildingSprite->setVisible(true);
    shadowSprite->setVisible(false);
    
    // 将建筑添加到地图
    map->addChild(this, 10); // 高层级显示
    
    CCLOG("Building placed successfully at (%.1f, %.1f)", position.x, position.y);
}

void Building::updateAppearance()
{
    // 更新建筑外观，比如根据等级改变颜色等
    if (buildingSprite && inPlaceMode) {
        updatePlacePreview(false); // 先假设不能放置
    }
}

void Building::updatePlacePreview(bool canPlace)
{
    if (!shadowSprite || !inPlaceMode) return;
    
    // 根据是否可以放置，改变预览颜色
    if (canPlace) {
        shadowSprite->setColor(Color3B::GREEN);  // 绿色表示可以放置
    } else {
        shadowSprite->setColor(Color3B::RED);    // 红色表示不能放置
    }
}