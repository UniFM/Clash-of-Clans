/*************************************************************
* @file     : TownHall.cpp
* @function ：大本营建筑实现 - 三菜单功能（信息/无条件升级/删除）
* @author   : 齐颖
* @note     : 1.升级无条件，无金币圣水校验；2.短击信息显图、短击取消删建筑；3.保留等级1-3切换逻辑
* @update   : 适配部落冲突标准 → 4x4网格尺寸，完美兼容瓦片地图
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 核心修改：适配部落冲突4x4网格 =====================
// 大本营网格尺寸（部落冲突标准：4x4网格，适配瓦片地图）
#define TOWN_HALL_GRID_SIZE     Size(4, 4)
// 单瓦片像素尺寸（根据你的地图配置填写，示例32px/格，可自行修改）
#define MAP_TILE_PIXEL_SIZE     41.0f
// 推导实际像素尺寸 = 网格数 × 单瓦片像素尺寸（4×4×41=164px，与原尺寸一致，无缝兼容）
#define TOWN_HALL_SIZE          Size(TOWN_HALL_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     TOWN_HALL_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// 大本营资源配置（原有不变）
#define TOWN_HALL_LEVEL_1_RES "buildings/Town_Hall1.png"
#define TOWN_HALL_LEVEL_2_RES "buildings/Town_Hall2.png"
#define TOWN_HALL_LEVEL_3_RES "buildings/Town_Hall3.png"
#define TOWN_HALL_INFO_LV1 "buildings/Town_Hall_Info1.png"
#define TOWN_HALL_INFO_LV2 "buildings/Town_Hall_Info2.png"
#define TOWN_HALL_INFO_LV3 "buildings/Town_Hall_Info3.png"
#define INFO_IMAGE_SCALE 0.8f

// ------------------------ 大本营子类实现 ------------------------
TownHall* TownHall::create(int level) {
    TownHall* townHall = new (std::nothrow) TownHall();
    if (townHall && townHall->init(BuildingType::TOWN_HALL, level)) {
        townHall->autorelease();
        CCLOG("【大本营】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            TOWN_HALL_GRID_SIZE.width, TOWN_HALL_GRID_SIZE.height);
        return townHall;
    }
    CC_SAFE_DELETE(townHall);
    return nullptr;
}

bool TownHall::loadBuildingRes() {
    return loadTownHallResByLevel(_level);
}

bool TownHall::loadTownHallResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = TOWN_HALL_LEVEL_1_RES; break;
    case 2: resPath = TOWN_HALL_LEVEL_2_RES; break;
    case 3: resPath = TOWN_HALL_LEVEL_3_RES; break;
    default: resPath = TOWN_HALL_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【大本营】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = TOWN_HALL_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    // 关键补充：配置菜单+信息图偏移量（适配4x4网格尺寸，与原有交互一致）
    m_menuOffset = Vec2(80, TOWN_HALL_SIZE.height / 2 - 140);
    m_infoImageOffset = Vec2(0, 60);

    return true;
}

bool TownHall::loadBuildingInfoRes() {
    return loadTownHallInfoResByLevel(_level);
}

bool TownHall::loadTownHallInfoResByLevel(int level) {
    // 销毁旧信息图，避免内存泄漏
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = TOWN_HALL_INFO_LV1; break;
    case 2: resPath = TOWN_HALL_INFO_LV2; break;
    case 3: resPath = TOWN_HALL_INFO_LV3; break;
    default: resPath = TOWN_HALL_INFO_LV1; break;
    }

    // 加载信息图，失败则创建红色调试块兜底
    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【大本营】等级%d信息图加载失败，使用红色调试块", level);
        _infoImage = Sprite::create();
        _infoImage->setColor(Color3B::RED);
        _infoImage->setContentSize(Size(300, 200));
    }

    // 信息图样式配置
    _infoImage->setAnchorPoint(Vec2(0.5f, 0.5f));
    _infoImage->setScale(INFO_IMAGE_SCALE);
    _infoImage->getTexture()->setAntiAliasTexParameters();

    // 挂载到信息层
    if (_infoLayer && _infoImage->getParent() == nullptr) {
        _infoLayer->addChild(_infoImage);
    }
    return true;
}

//  核心重写：带金币校验+分级扣费的升级逻辑（无弹窗、静默拦截）
bool TownHall::upgradeBuilding() {
    // 1. 最高等级校验（保留原有逻辑：等级上限3级）
    if (_level >= 3) {
        CCLOG("【大本营】已达最高等级3，无法升级");
        return false;
    }

    //  2. 分级获取升级所需金币 & 金币校验（核心新增）
    int costGold = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【大本营】升级失败：游戏管理器实例为空");
        return false;
    }

    //  分级扣费规则：1→2扣1000金币，2→3扣4000金币
    switch (_level) {
    case 1: costGold = 1000; break; // Lv1升Lv2，消耗1000金币
    case 2: costGold = 4000; break; // Lv2升Lv3，消耗4000金币
    default: return false;
    }

    //  金币不足：直接禁止升级，无弹窗、无提示，静默返回
    if (!gm->spendResource(ResourceType::GOLD, costGold)) {
        CCLOG("【大本营】升级失败：金币不足！当前%d，所需%d", gm->getResource(ResourceType::GOLD), costGold);
        return false;
    }

    SceneMap::getInstance()->refreshResourceImmediately();

    // 3. 金币扣除成功，执行升级逻辑（保留原有所有功能）
    int oldLevel = _level;
    _level++;

    // 4. 加载对应等级的建筑图+信息图
    bool buildingUpgradeSuccess = loadTownHallResByLevel(_level);
    bool infoUpgradeSuccess = loadTownHallInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【大本营】升级成功：Lv%d → Lv%d | 扣除金币%d，剩余%d",
            oldLevel, _level, costGold, gm->getResource(ResourceType::GOLD));

        // 升级后自动关闭菜单+隐藏信息图，恢复初始状态（保留原有逻辑）
        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        if (!infoUpgradeSuccess) {
            CCLOG("【大本营】建筑升级成功，但信息图更新失败");
            //  新增：刷新金币总上限（金矿升级，容量上涨）
            SceneMap::getInstance()->calculateTotalGoldCapacity();
        }
        return true;
    }
    else {
        // 升级失败回滚等级（保留原有逻辑）
        _level = oldLevel;
        //  额外兜底：升级失败时，返还扣除的金币
        gm->addResource(ResourceType::GOLD, costGold);
        CCLOG("【大本营】升级失败，等级回滚至Lv%d，返还金币%d", oldLevel, costGold);
        return false;
    }
}

// ------------------------ 全局创建大本营（入口方法，无改动） ------------------------
void createLevel1TownHall() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建大本营】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1TownHall();
            });
        return;
    }

    TownHall* townHall = TownHall::create(1);
    if (!townHall) {
        CCLOG("【创建大本营】创建实例失败");
        return;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 screenCenter = Vec2(visibleSize.width / 2, visibleSize.height / 2);
    townHall->setBuildingPosition(screenCenter);

    homeMap->addChild(townHall, 200);
    CCLOG("【创建大本营】1级大本营创建成功，位置：(%.0f, %.0f) | 4x4网格适配完成",
        screenCenter.x, screenCenter.y);
}