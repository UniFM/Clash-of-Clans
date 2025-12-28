/*************************************************************
* @file     : Barracks.cpp
* @function ：兵营建筑实现 - 圣水扣费升级+等级校验+纯功能建筑
* @note     : 1.1→2级耗2000圣水、2→3级耗10000圣水；2.圣水不足禁止升级，无弹窗；3.等级1-3切换；
*             4.纯功能建筑，不影响金币/圣水存储上限、产速更新；适配瓦片地图+2x2网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 兵营网格/尺寸配置 =====================
#define BARRACKS_GRID_SIZE      Size(2, 2)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define BARRACKS_SIZE           Size(BARRACKS_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     BARRACKS_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 兵营资源路径配置 =====================
#define BARRACKS_LEVEL_1_RES    "buildings/Barracks1.png"
#define BARRACKS_LEVEL_2_RES    "buildings/Barracks2.png"
#define BARRACKS_LEVEL_3_RES    "buildings/Barracks3.png"
#define BARRACKS_INFO_LV1       "buildings/Barracks_Info1.png"
#define BARRACKS_INFO_LV2       "buildings/Barracks_Info2.png"
#define BARRACKS_INFO_LV3       "buildings/Barracks_Info3.png"
#define INFO_IMAGE_SCALE        0.8f

// ------------------------ 兵营子类实现 ------------------------
Barracks* Barracks::create(int level) {
    Barracks* barracks = new (std::nothrow) Barracks();
    if (barracks && barracks->init(BuildingType::BARRACKS, level)) {
        barracks->autorelease();
        CCLOG("【兵营】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            BARRACKS_GRID_SIZE.width, BARRACKS_GRID_SIZE.height);
        return barracks;
    }
    CC_SAFE_DELETE(barracks);
    return nullptr;
}

bool Barracks::loadBuildingRes() {
    return loadBarracksResByLevel(_level);
}

bool Barracks::loadBarracksResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = BARRACKS_LEVEL_1_RES; break;
    case 2: resPath = BARRACKS_LEVEL_2_RES; break;
    case 3: resPath = BARRACKS_LEVEL_3_RES; break;
    default: resPath = BARRACKS_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【兵营】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = BARRACKS_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(40, BARRACKS_SIZE.height / 2 - 120);
    m_infoImageOffset = Vec2(0, 30);

    return true;
}

bool Barracks::loadBuildingInfoRes() {
    return loadBarracksInfoResByLevel(_level);
}

bool Barracks::loadBarracksInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = BARRACKS_INFO_LV1; break;
    case 2: resPath = BARRACKS_INFO_LV2; break;
    case 3: resPath = BARRACKS_INFO_LV3; break;
    default: resPath = BARRACKS_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【兵营】等级%d信息图加载失败，使用红色调试块", level);
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

// 核心升级逻辑（仅扣圣水，不影响存储/产速，精准匹配消耗要求）
bool Barracks::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【兵营】已达最高等级3，无法升级");
        return false;
    }

    int costElixir = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【兵营】升级失败：游戏管理器实例为空");
        return false;
    }

    // 兵营升级消耗配置：1→2=2000圣水，2→3=10000圣水
    switch (_level) {
    case 1: costElixir = 2000; break;
    case 2: costElixir = 10000; break;
    default: return false;
    }

    if (!gm->spendResource(ResourceType::ELIXIR, costElixir)) {
        CCLOG("【兵营】升级失败：圣水不足！当前%d，所需%d", gm->getResource(ResourceType::ELIXIR), costElixir);
        return false;
    }

    // 仅刷新资源UI，不更新存储上限、不联动产速（核心要求）
    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadBarracksResByLevel(_level);
    bool infoUpgradeSuccess = loadBarracksInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【兵营】升级成功：Lv%d → Lv%d | 扣除圣水%d，剩余%d",
            oldLevel, _level, costElixir, gm->getResource(ResourceType::ELIXIR));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        if (!infoUpgradeSuccess) {
            CCLOG("【兵营】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::ELIXIR, costElixir);
        CCLOG("【兵营】升级失败，等级回滚至Lv%d，返还圣水%d", oldLevel, costElixir);
        return false;
    }
}

// ------------------------ 全局创建兵营入口 ------------------------
void createLevel1Barracks() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建兵营】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1Barracks();
            });
        return;
    }

    Barracks* barracks = Barracks::create(1);
    if (!barracks) {
        CCLOG("【创建兵营】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.6f);
    barracks->setBuildingPosition(spawnPos);

    homeMap->addChild(barracks, 200);
    CCLOG("【创建兵营】1级兵营创建成功，位置：(%.0f, %.0f) | 2x2网格适配完成",
        spawnPos.x, spawnPos.y);
}