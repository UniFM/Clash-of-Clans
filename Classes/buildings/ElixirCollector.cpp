/*************************************************************
* @file     : ElixirCollector.cpp
* @function ：圣水收集器实现 - 金币扣费升级+等级校验+产速联动
* @note     : 1.1→2级耗300金币、2→3级耗700金币；2.金币不足禁止升级；3.等级1-3切换；
*             4.升级后自动刷新全局产速，完美适配瓦片地图+2x2网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 圣水收集器网格/尺寸配置（与金矿完全一致）=====================
#define ELIXIR_COLLECTOR_GRID_SIZE Size(2, 2)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define ELIXIR_COLLECTOR_SIZE   Size(ELIXIR_COLLECTOR_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     ELIXIR_COLLECTOR_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 圣水收集器资源路径配置 =====================
#define ELIXIR_COLLECTOR_LEVEL_1_RES   "buildings/Elixir_Collector1.png"
#define ELIXIR_COLLECTOR_LEVEL_2_RES   "buildings/Elixir_Collector2.png"
#define ELIXIR_COLLECTOR_LEVEL_3_RES   "buildings/Elixir_Collector3.png"
#define ELIXIR_COLLECTOR_INFO_LV1      "buildings/Elixir_Collector_Info1.png"
#define ELIXIR_COLLECTOR_INFO_LV2      "buildings/Elixir_Collector_Info2.png"
#define ELIXIR_COLLECTOR_INFO_LV3      "buildings/Elixir_Collector_Info3.png"
#define INFO_IMAGE_SCALE        0.8f

// ------------------------ 圣水收集器子类实现 ------------------------
ElixirCollector* ElixirCollector::create(int level) {
    ElixirCollector* elixirCollector = new (std::nothrow) ElixirCollector();
    if (elixirCollector && elixirCollector->init(BuildingType::ELIXIR_COLLECTOR, level)) {
        elixirCollector->autorelease();
        CCLOG("【圣水收集器】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            ELIXIR_COLLECTOR_GRID_SIZE.width, ELIXIR_COLLECTOR_GRID_SIZE.height);
        return elixirCollector;
    }
    CC_SAFE_DELETE(elixirCollector);
    return nullptr;
}

bool ElixirCollector::loadBuildingRes() {
    return loadElixirCollectorResByLevel(_level);
}

bool ElixirCollector::loadElixirCollectorResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = ELIXIR_COLLECTOR_LEVEL_1_RES; break;
    case 2: resPath = ELIXIR_COLLECTOR_LEVEL_2_RES; break;
    case 3: resPath = ELIXIR_COLLECTOR_LEVEL_3_RES; break;
    default: resPath = ELIXIR_COLLECTOR_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【圣水收集器】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = ELIXIR_COLLECTOR_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(40, ELIXIR_COLLECTOR_SIZE.height / 2 - 120);
    m_infoImageOffset = Vec2(0, 30);

    return true;
}

bool ElixirCollector::loadBuildingInfoRes() {
    return loadElixirCollectorInfoResByLevel(_level);
}

bool ElixirCollector::loadElixirCollectorInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = ELIXIR_COLLECTOR_INFO_LV1; break;
    case 2: resPath = ELIXIR_COLLECTOR_INFO_LV2; break;
    case 3: resPath = ELIXIR_COLLECTOR_INFO_LV3; break;
    default: resPath = ELIXIR_COLLECTOR_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【圣水收集器】等级%d信息图加载失败，使用蓝色调试块", level);
        _infoImage = Sprite::create();
        _infoImage->setColor(Color3B::BLUE);
        _infoImage->setContentSize(Size(250, 180));
    }

    _infoImage->setAnchorPoint(Vec2(0.5f, 0.5f));
    _infoImage->setScale(INFO_IMAGE_SCALE);
    _infoImage->getTexture()->setAntiAliasTexParameters();

    if (_infoLayer && _infoImage->getParent() == nullptr) {
        _infoLayer->addChild(_infoImage);
    }
    return true;
}

//  核心升级逻辑（与金矿对称：扣金币、升级后刷新圣水产速）
bool ElixirCollector::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【圣水收集器】已达最高等级3，无法升级");
        return false;
    }

    int costGold = 0; //  升级消耗金币（与金矿耗圣水对称）
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【圣水收集器】升级失败：游戏管理器实例为空");
        return false;
    }

    switch (_level) {
    case 1: costGold = 300; break;  // 1→2 耗300金币
    case 2: costGold = 700; break;  // 2→3 耗700金币
    default: return false;
    }

    if (!gm->spendResource(ResourceType::GOLD, costGold)) { //  校验金币
        CCLOG("【圣水收集器】升级失败：金币不足！当前%d，所需%d", gm->getResource(ResourceType::GOLD), costGold);
        return false;
    }

    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadElixirCollectorResByLevel(_level);
    bool infoUpgradeSuccess = loadElixirCollectorInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【圣水收集器】升级成功：Lv%d → Lv%d | 扣除金币%d，剩余%d",
            oldLevel, _level, costGold, gm->getResource(ResourceType::GOLD));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        //  强制刷新圣水产速+圣水总容量（核心联动）
        SceneMap::getInstance()->calculateTotalElixirProduceSpeed();
        SceneMap::getInstance()->calculateTotalElixirCapacity();

        if (!infoUpgradeSuccess) {
            CCLOG("【圣水收集器】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::GOLD, costGold); //  返还金币
        CCLOG("【圣水收集器】升级失败，等级回滚至Lv%d，返还金币%d", oldLevel, costGold);
        return false;
    }
}

// ------------------------ 全局创建圣水收集器入口 ------------------------
void createLevel1ElixirCollector() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建圣水收集器】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1ElixirCollector();
            });
        return;
    }

    ElixirCollector* elixirCollector = ElixirCollector::create(1);
    if (!elixirCollector) {
        CCLOG("【创建圣水收集器】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.2f, visibleSize.height * 0.6f); // 与金矿位置上下区分
    elixirCollector->setBuildingPosition(spawnPos);

    homeMap->addChild(elixirCollector, 200);
    CCLOG("【创建圣水收集器】1级圣水收集器创建成功，位置：(%.0f, %.0f) | 2x2网格适配完成",
        spawnPos.x, spawnPos.y);
}