/*************************************************************
* @file     : Cannon.cpp
* @function ：加农炮建筑实现 - 金币扣费升级+等级校验+纯防御建筑
* @note     : 1.1→2级耗1000金币、2→3级耗4000金币；2.金币不足禁止升级，无弹窗；3.等级1-3切换；
*             4.纯防御建筑，不影响金币/圣水存储上限、产速更新；适配瓦片地图+2x2网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 加农炮网格/尺寸配置 =====================
#define CANNON_GRID_SIZE        Size(2, 2)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define CANNON_SIZE             Size(CANNON_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     CANNON_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 加农炮资源路径配置 =====================
#define CANNON_LEVEL_1_RES      "buildings/Cannon1.png"
#define CANNON_LEVEL_2_RES      "buildings/Cannon2.png"
#define CANNON_LEVEL_3_RES      "buildings/Cannon3.png"
#define CANNON_INFO_LV1         "buildings/Cannon_Info1.png"
#define CANNON_INFO_LV2         "buildings/Cannon_Info2.png"
#define CANNON_INFO_LV3         "buildings/Cannon_Info3.png"
#define INFO_IMAGE_SCALE        0.8f

// ------------------------ 加农炮子类实现 ------------------------
Cannon* Cannon::create(int level) {
    Cannon* cannon = new (std::nothrow) Cannon();
    if (cannon && cannon->init(BuildingType::CANNON, level)) {
        cannon->autorelease();
        CCLOG("【加农炮】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            CANNON_GRID_SIZE.width, CANNON_GRID_SIZE.height);
        return cannon;
    }
    CC_SAFE_DELETE(cannon);
    return nullptr;
}

bool Cannon::loadBuildingRes() {
    return loadCannonResByLevel(_level);
}

bool Cannon::loadCannonResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = CANNON_LEVEL_1_RES; break;
    case 2: resPath = CANNON_LEVEL_2_RES; break;
    case 3: resPath = CANNON_LEVEL_3_RES; break;
    default: resPath = CANNON_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【加农炮】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = CANNON_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(40, CANNON_SIZE.height / 2 - 120);
    m_infoImageOffset = Vec2(0, 30);

    return true;
}

bool Cannon::loadBuildingInfoRes() {
    return loadCannonInfoResByLevel(_level);
}

bool Cannon::loadCannonInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = CANNON_INFO_LV1; break;
    case 2: resPath = CANNON_INFO_LV2; break;
    case 3: resPath = CANNON_INFO_LV3; break;
    default: resPath = CANNON_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【加农炮】等级%d信息图加载失败，使用红色调试块", level);
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
bool Cannon::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【加农炮】已达最高等级3，无法升级");
        return false;
    }

    int costGold = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【加农炮】升级失败：游戏管理器实例为空");
        return false;
    }

    // 加农炮升级消耗配置：1→2=1000金币，2→3=4000金币
    switch (_level) {
    case 1: costGold = 1000; break;
    case 2: costGold = 4000; break;
    default: return false;
    }

    if (!gm->spendResource(ResourceType::GOLD, costGold)) {
        CCLOG("【加农炮】升级失败：金币不足！当前%d，所需%d", gm->getResource(ResourceType::GOLD), costGold);
        return false;
    }

    // 仅刷新资源UI，不更新存储上限、不联动产速
    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadCannonResByLevel(_level);
    bool infoUpgradeSuccess = loadCannonInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【加农炮】升级成功：Lv%d → Lv%d | 扣除金币%d，剩余%d",
            oldLevel, _level, costGold, gm->getResource(ResourceType::GOLD));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        if (!infoUpgradeSuccess) {
            CCLOG("【加农炮】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::GOLD, costGold);
        CCLOG("【加农炮】升级失败，等级回滚至Lv%d，返还金币%d", oldLevel, costGold);
        return false;
    }
}

// ------------------------ 全局创建加农炮入口 ------------------------
void createLevel1Cannon() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建加农炮】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1Cannon();
            });
        return;
    }

    Cannon* cannon = Cannon::create(1);
    if (!cannon) {
        CCLOG("【创建加农炮】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.25f, visibleSize.height * 0.4f);
    cannon->setBuildingPosition(spawnPos);

    homeMap->addChild(cannon, 200);
    CCLOG("【创建加农炮】1级加农炮创建成功，位置：(%.0f, %.0f) | 2x2网格适配完成",
        spawnPos.x, spawnPos.y);
}