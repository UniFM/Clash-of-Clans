/*************************************************************
* @file     : ArcherTower.cpp
* @function ：箭塔建筑实现 - 金币扣费升级+等级校验+纯防御建筑
* @note     : 1.1→2级耗2000金币、2→3级耗5000金币；2.金币不足禁止升级，无弹窗；3.等级1-3切换；
*             4.纯防御建筑，不影响金币/圣水存储上限、产速更新；适配瓦片地图+2x2网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 箭塔网格/尺寸配置 =====================
#define ARCHER_TOWER_GRID_SIZE  Size(2, 2)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define ARCHER_TOWER_SIZE       Size(ARCHER_TOWER_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     ARCHER_TOWER_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 箭塔资源路径配置 =====================
#define ARCHER_TOWER_LEVEL_1_RES "buildings/Archer_Tower1.png"
#define ARCHER_TOWER_LEVEL_2_RES "buildings/Archer_Tower2.png"
#define ARCHER_TOWER_LEVEL_3_RES "buildings/Archer_Tower3.png"
#define ARCHER_TOWER_INFO_LV1    "buildings/ArcherTower_Info1.png"
#define ARCHER_TOWER_INFO_LV2    "buildings/ArcherTower_Info2.png"
#define ARCHER_TOWER_INFO_LV3    "buildings/ArcherTower_Info3.png"
#define INFO_IMAGE_SCALE         0.8f

// ------------------------ 箭塔子类实现 ------------------------
ArcherTower* ArcherTower::create(int level) {
    ArcherTower* archerTower = new (std::nothrow) ArcherTower();
    if (archerTower && archerTower->init(BuildingType::ARCHER_TOWER, level)) {
        archerTower->autorelease();
        CCLOG("【箭塔】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            ARCHER_TOWER_GRID_SIZE.width, ARCHER_TOWER_GRID_SIZE.height);
        return archerTower;
    }
    CC_SAFE_DELETE(archerTower);
    return nullptr;
}

bool ArcherTower::loadBuildingRes() {
    return loadArcherTowerResByLevel(_level);
}

bool ArcherTower::loadArcherTowerResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = ARCHER_TOWER_LEVEL_1_RES; break;
    case 2: resPath = ARCHER_TOWER_LEVEL_2_RES; break;
    case 3: resPath = ARCHER_TOWER_LEVEL_3_RES; break;
    default: resPath = ARCHER_TOWER_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【箭塔】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = ARCHER_TOWER_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(40, ARCHER_TOWER_SIZE.height / 2 - 120);
    m_infoImageOffset = Vec2(0, 30);

    return true;
}

bool ArcherTower::loadBuildingInfoRes() {
    return loadArcherTowerInfoResByLevel(_level);
}

bool ArcherTower::loadArcherTowerInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = ARCHER_TOWER_INFO_LV1; break;
    case 2: resPath = ARCHER_TOWER_INFO_LV2; break;
    case 3: resPath = ARCHER_TOWER_INFO_LV3; break;
    default: resPath = ARCHER_TOWER_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【箭塔】等级%d信息图加载失败，使用红色调试块", level);
        _infoImage = Sprite::create();
        _infoImage->setColor(Color3B::RED);
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

// 核心升级逻辑（仅扣金币，不影响存储/产速）
bool ArcherTower::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【箭塔】已达最高等级3，无法升级");
        return false;
    }

    int costGold = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【箭塔】升级失败：游戏管理器实例为空");
        return false;
    }

    // 箭塔升级消耗配置：1→2=2000金币，2→3=5000金币
    switch (_level) {
    case 1: costGold = 2000; break;
    case 2: costGold = 5000; break;
    default: return false;
    }

    if (!gm->spendResource(ResourceType::GOLD, costGold)) {
        CCLOG("【箭塔】升级失败：金币不足！当前%d，所需%d", gm->getResource(ResourceType::GOLD), costGold);
        return false;
    }

    // 仅刷新资源UI，不更新存储上限、不联动产速
    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadArcherTowerResByLevel(_level);
    bool infoUpgradeSuccess = loadArcherTowerInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【箭塔】升级成功：Lv%d → Lv%d | 扣除金币%d，剩余%d",
            oldLevel, _level, costGold, gm->getResource(ResourceType::GOLD));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        if (!infoUpgradeSuccess) {
            CCLOG("【箭塔】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::GOLD, costGold);
        CCLOG("【箭塔】升级失败，等级回滚至Lv%d，返还金币%d", oldLevel, costGold);
        return false;
    }
}

// ------------------------ 全局创建箭塔入口 ------------------------
void createLevel1ArcherTower() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建箭塔】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1ArcherTower();
            });
        return;
    }

    ArcherTower* archerTower = ArcherTower::create(1);
    if (!archerTower) {
        CCLOG("【创建箭塔】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.75f, visibleSize.height * 0.4f);
    archerTower->setBuildingPosition(spawnPos);

    homeMap->addChild(archerTower, 200);
    CCLOG("【创建箭塔】1级箭塔创建成功，位置：(%.0f, %.0f) | 2x2网格适配完成",
        spawnPos.x, spawnPos.y);
}