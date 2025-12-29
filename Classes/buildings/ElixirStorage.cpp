/*************************************************************
* @file     : ElixirStorage.cpp
* @function ：圣水瓶建筑实现 - 金币扣费升级+等级校验
* @author   : 齐颖
* @note     : 1.1→2级耗750金币、2→3级耗1500金币；2.金币不足禁止升级；3.等级1-3切换；
*             4.复刻储金罐逻辑，完美兼容瓦片地图+3x3网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 圣水瓶网格/尺寸配置（与储金罐完全一致）=====================
#define ELIXIR_STORAGE_GRID_SIZE   Size(3, 3)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define ELIXIR_STORAGE_SIZE     Size(ELIXIR_STORAGE_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     ELIXIR_STORAGE_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 圣水瓶资源路径配置 =====================
#define ELIXIR_STORAGE_LEVEL_1_RES "buildings/Elixir_Storage1.png"
#define ELIXIR_STORAGE_LEVEL_2_RES "buildings/Elixir_Storage2.png"
#define ELIXIR_STORAGE_LEVEL_3_RES "buildings/Elixir_Storage3.png"
#define ELIXIR_STORAGE_INFO_LV1    "buildings/Elixir_Storage_Info1.png"
#define ELIXIR_STORAGE_INFO_LV2    "buildings/Elixir_Storage_Info2.png"
#define ELIXIR_STORAGE_INFO_LV3    "buildings/Elixir_Storage_Info3.png"
#define INFO_IMAGE_SCALE         0.8f

// ------------------------ 圣水瓶子类实现 ------------------------
ElixirStorage* ElixirStorage::create(int level) {
    ElixirStorage* elixirStorage = new (std::nothrow) ElixirStorage();
    if (elixirStorage && elixirStorage->init(BuildingType::ELIXIR_STORAGE, level)) {
        elixirStorage->autorelease();
        CCLOG("【圣水瓶】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            ELIXIR_STORAGE_GRID_SIZE.width, ELIXIR_STORAGE_GRID_SIZE.height);
        return elixirStorage;
    }
    CC_SAFE_DELETE(elixirStorage);
    return nullptr;
}

bool ElixirStorage::loadBuildingRes() {
    return loadElixirStorageResByLevel(_level);
}

bool ElixirStorage::loadElixirStorageResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = ELIXIR_STORAGE_LEVEL_1_RES; break;
    case 2: resPath = ELIXIR_STORAGE_LEVEL_2_RES; break;
    case 3: resPath = ELIXIR_STORAGE_LEVEL_3_RES; break;
    default: resPath = ELIXIR_STORAGE_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【圣水瓶】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = ELIXIR_STORAGE_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(60, ELIXIR_STORAGE_SIZE.height / 2 - 105);
    m_infoImageOffset = Vec2(0, 45);

    return true;
}

bool ElixirStorage::loadBuildingInfoRes() {
    return loadElixirStorageInfoResByLevel(_level);
}

bool ElixirStorage::loadElixirStorageInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = ELIXIR_STORAGE_INFO_LV1; break;
    case 2: resPath = ELIXIR_STORAGE_INFO_LV2; break;
    case 3: resPath = ELIXIR_STORAGE_INFO_LV3; break;
    default: resPath = ELIXIR_STORAGE_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【圣水瓶】等级%d信息图加载失败，使用蓝色调试块", level);
        _infoImage = Sprite::create();
        _infoImage->setColor(Color3B::BLUE);
        _infoImage->setContentSize(Size(280, 190));
    }

    _infoImage->setAnchorPoint(Vec2(0.5f, 0.5f));
    _infoImage->setScale(INFO_IMAGE_SCALE);
    _infoImage->getTexture()->setAntiAliasTexParameters();

    if (_infoLayer && _infoImage->getParent() == nullptr) {
        _infoLayer->addChild(_infoImage);
    }
    return true;
}

//  核心升级逻辑（与储金罐对称：扣金币、升级后刷新圣水容量）
bool ElixirStorage::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【圣水瓶】已达最高等级3，无法升级");
        return false;
    }

    int costGold = 0; //  升级消耗金币（与储金罐耗圣水对称）
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【圣水瓶】升级失败：游戏管理器实例为空");
        return false;
    }

    switch (_level) {
    case 1: costGold = 750;  break;  // 1→2 耗750金币
    case 2: costGold = 1500; break;  // 2→3 耗1500金币
    default: return false;
    }

    if (!gm->spendResource(ResourceType::GOLD, costGold)) { //  校验金币
        CCLOG("【圣水瓶】升级失败：金币不足！当前%d，所需%d", gm->getResource(ResourceType::GOLD), costGold);
        return false;
    }

    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadElixirStorageResByLevel(_level);
    bool infoUpgradeSuccess = loadElixirStorageInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【圣水瓶】升级成功：Lv%d → Lv%d | 扣除金币%d，剩余%d",
            oldLevel, _level, costGold, gm->getResource(ResourceType::GOLD));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        //  刷新圣水总容量（核心联动）
        SceneMap::getInstance()->calculateTotalElixirCapacity();

        if (!infoUpgradeSuccess) {
            CCLOG("【圣水瓶】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::GOLD, costGold); //  返还金币
        CCLOG("【圣水瓶】升级失败，等级回滚至Lv%d，返还金币%d", oldLevel, costGold);
        return false;
    }
}

// ------------------------ 全局创建圣水瓶入口 ------------------------
void createLevel1ElixirStorage() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建圣水瓶】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1ElixirStorage();
            });
        return;
    }

    ElixirStorage* elixirStorage = ElixirStorage::create(1);
    if (!elixirStorage) {
        CCLOG("【创建圣水瓶】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.8f, visibleSize.height * 0.6f); // 与储金罐位置上下区分
    elixirStorage->setBuildingPosition(spawnPos);

    homeMap->addChild(elixirStorage, 200);
    CCLOG("【创建圣水瓶】1级圣水瓶创建成功，位置：(%.0f, %.0f) | 3x3网格适配完成",
        spawnPos.x, spawnPos.y);
}