#include "HomeVillageMap.h"
#include "cocos2d.h"
#include "Troops/Unit.h"
#include "Troops/TroopManager.h"
#include "Troops/TroopDefinitions.h"
#include "SceneMap.h"

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

    // 初始化时更新一次部队显示
    this->updateTroopDisplay();

    return true;
}



// [新增] 移植自 SceneMap 的 updateTroopDisplay 函数
void HomeVillageMap::updateTroopDisplay() {
    log("HomeVillageMap::updateTroopDisplay called");

    // 获取BaseMap中的部队容器
    auto troopsContainer = this->getTroopsContainer();
    
    // 容错处理
    if (!troopsContainer) {
        log("ERROR: troopsContainer was NULL in BaseMap.");
        return;
    }

    // 1. 清理旧显示
    troopsContainer->removeAllChildren();

    // 2. 获取草地区域内的1/4位置作为兵营位置（相对于地图内容固定）
    // 这里使用相对于地图的局部坐标，让兵营随地图移动
    // 计算草地层1/4位置的绝对坐标（相对于草地层自身）
    float grassRelativeX = BaseMap::getGrassRectWidth() * 0.5f;
    float grassRelativeY = BaseMap::getGrassRectHeight() * 0.3f;

    // 转换为世界坐标系（考虑草地层自身的原点）
    float worldX = BaseMap::getGrassOffsetX() + grassRelativeX;
    float worldY = BaseMap::getGrassOffsetY() + grassRelativeY;

    // 设置兵营位置（使用地图局部坐标系）
    Vec2 barracksPos = Vec2(worldX, worldY);

    // 游荡半径
    float wanderRadius = 100.0f;

    // 3. 从 TroopManager 获取实际选中的部队数量
    auto tm = TroopManager::getInstance();

    std::vector<UnitType> types = {
        UnitType::BARBARIAN,
        UnitType::ARCHER,
        UnitType::GOBLIN,
        UnitType::GIANT
    };

    int totalSpawned = 0;

    // 4. 生成并显示
    for (auto type : types) {
        // 转换类型
        TroopType troopType = static_cast<TroopType>(type);

        int count = tm->getTroopCount(troopType);

        if (count > 0) {
            log("Spawning type %d: count = %d", (int)type, count);
        }

        for (int i = 0; i < count; ++i) {
            Unit* unit = Unit::create(type);

            if (unit) {
                // 随机初始位置（相对于地图的局部坐标）
                float angle = CCRANDOM_0_1() * M_PI * 2;
                float dist = CCRANDOM_0_1() * wanderRadius;
                Vec2 startPos = barracksPos + Vec2(cos(angle) * dist, sin(angle) * dist);

                unit->setPosition(startPos);

                // 添加到容器（容器是地图的子节点，所以使用地图的局部坐标系）
                troopsContainer->addChild(unit);

                // 启动游荡
                unit->wanderAround(barracksPos, wanderRadius);

                totalSpawned++;
            }
        }
    }
    log("HomeVillageMap::updateTroopDisplay finished. Spawned %d units.", totalSpawned);
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
