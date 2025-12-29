/*************************************************************
* @file     : GoldMine.cpp
* @function ：金矿建筑实现 - 圣水扣费升级+等级校验+产速联动
* @note     : 1.1→2级耗300圣水、2→3级耗700圣水；2.圣水不足禁止升级，无弹窗；3.等级1-3切换；
*             4.升级后自动刷新全局产速，完美适配瓦片地图+2x2网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 金矿网格/尺寸配置 =====================
#define GOLD_MINE_GRID_SIZE     Size(2, 2)
#define MAP_TILE_PIXEL_SIZE     41.0f
#define GOLD_MINE_SIZE          Size(GOLD_MINE_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     GOLD_MINE_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 金矿资源路径配置 =====================
#define GOLD_MINE_LEVEL_1_RES   "buildings/Gold_Mine1.png"
#define GOLD_MINE_LEVEL_2_RES   "buildings/Gold_Mine2.png"
#define GOLD_MINE_LEVEL_3_RES   "buildings/Gold_Mine3.png"
#define GOLD_MINE_INFO_LV1      "buildings/Gold_Mine_Info1.png"
#define GOLD_MINE_INFO_LV2      "buildings/Gold_Mine_Info2.png"
#define GOLD_MINE_INFO_LV3      "buildings/Gold_Mine_Info3.png"
#define INFO_IMAGE_SCALE        0.8f

// ------------------------ 金矿子类实现 ------------------------
GoldMine* GoldMine::create(int level) {
    GoldMine* goldMine = new (std::nothrow) GoldMine();
    if (goldMine && goldMine->init(BuildingType::GOLD_MINE, level)) {
        goldMine->autorelease();
        CCLOG("【金矿】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            GOLD_MINE_GRID_SIZE.width, GOLD_MINE_GRID_SIZE.height);
        return goldMine;
    }
    CC_SAFE_DELETE(goldMine);
    return nullptr;
}

bool GoldMine::loadBuildingRes() {
    return loadGoldMineResByLevel(_level);
}

bool GoldMine::loadGoldMineResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = GOLD_MINE_LEVEL_1_RES; break;
    case 2: resPath = GOLD_MINE_LEVEL_2_RES; break;
    case 3: resPath = GOLD_MINE_LEVEL_3_RES; break;
    default: resPath = GOLD_MINE_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【金矿】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = GOLD_MINE_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    m_menuOffset = Vec2(40, GOLD_MINE_SIZE.height / 2 - 120);
    m_infoImageOffset = Vec2(0, 30);

    return true;
}

bool GoldMine::loadBuildingInfoRes() {
    return loadGoldMineInfoResByLevel(_level);
}

bool GoldMine::loadGoldMineInfoResByLevel(int level) {
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = GOLD_MINE_INFO_LV1; break;
    case 2: resPath = GOLD_MINE_INFO_LV2; break;
    case 3: resPath = GOLD_MINE_INFO_LV3; break;
    default: resPath = GOLD_MINE_INFO_LV1; break;
    }

    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【金矿】等级%d信息图加载失败，使用红色调试块", level);
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

//  核心升级逻辑（含产速联动）
bool GoldMine::upgradeBuilding() {
    if (_level >= 3) {
        CCLOG("【金矿】已达最高等级3，无法升级");
        return false;
    }

    int costElixir = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【金矿】升级失败：游戏管理器实例为空");
        return false;
    }

    switch (_level) {
    case 1: costElixir = 300; break;
    case 2: costElixir = 700; break;
    default: return false;
    }

    if (!gm->spendResource(ResourceType::ELIXIR, costElixir)) {
        CCLOG("【金矿】升级失败：圣水不足！当前%d，所需%d", gm->getResource(ResourceType::ELIXIR), costElixir);
        return false;
    }

    SceneMap::getInstance()->refreshResourceImmediately();

    int oldLevel = _level;
    _level++;

    bool buildingUpgradeSuccess = loadGoldMineResByLevel(_level);
    bool infoUpgradeSuccess = loadGoldMineInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【金矿】升级成功：Lv%d → Lv%d | 扣除圣水%d，剩余%d",
            oldLevel, _level, costElixir, gm->getResource(ResourceType::ELIXIR));

        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        //  强制刷新产速（核心加固，确保升级后产速即时生效）
        SceneMap::getInstance()->calculateTotalGoldProduceSpeed();
        //  新增：刷新金币总上限（金矿升级，容量上涨）
        SceneMap::getInstance()->calculateTotalGoldCapacity();

        if (!infoUpgradeSuccess) {
            CCLOG("【金矿】建筑升级成功，但信息图更新失败");
        }
        return true;
    }
    else {
        _level = oldLevel;
        gm->addResource(ResourceType::ELIXIR, costElixir);
        CCLOG("【金矿】升级失败，等级回滚至Lv%d，返还圣水%d", oldLevel, costElixir);
        return false;
    }
}

// ------------------------ 全局创建金矿入口 ------------------------
void createLevel1GoldMine() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建金矿】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1GoldMine();
            });
        return;
    }

    GoldMine* goldMine = GoldMine::create(1);
    if (!goldMine) {
        CCLOG("【创建金矿】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.2f, visibleSize.height * 0.3f);
    goldMine->setBuildingPosition(spawnPos);

    homeMap->addChild(goldMine, 200);
    CCLOG("【创建金矿】1级金矿创建成功，位置：(%.0f, %.0f) | 2x2网格适配完成",
        spawnPos.x, spawnPos.y);
}