/*************************************************************
* @file     : GoldStorage.cpp
* @function ：储金罐建筑实现 - 圣水扣费升级+等级校验
* @author   : 齐颖
* @note     : 1.1→2级耗750圣水、2→3级耗1500圣水；2.圣水不足禁止升级，无弹窗；3.等级1-3切换；
*             4.复刻大本营逻辑，完美兼容瓦片地图+3x3网格尺寸
**************************************************************/
#include "BuildingsData.h"
#include "Map/HomeVillageMap.h"
#include "Control/GameManager.h"
#include "Map/SceneMap.h"

USING_NS_CC;

// ===================== 储金罐网格/尺寸配置（适配你的地图）=====================
#define GOLD_STORAGE_GRID_SIZE   Size(3, 3)    // 储金罐网格尺寸（部落冲突标准3x3）
#define MAP_TILE_PIXEL_SIZE     41.0f         // 和大本营/金矿保持一致的瓦片像素尺寸
#define GOLD_STORAGE_SIZE       Size(GOLD_STORAGE_GRID_SIZE.width * MAP_TILE_PIXEL_SIZE, \
                                     GOLD_STORAGE_GRID_SIZE.height * MAP_TILE_PIXEL_SIZE)

// ===================== 储金罐资源路径配置（和大本营格式一致）=====================
#define GOLD_STORAGE_LEVEL_1_RES "buildings/Gold_Storage1.png"
#define GOLD_STORAGE_LEVEL_2_RES "buildings/Gold_Storage2.png"
#define GOLD_STORAGE_LEVEL_3_RES "buildings/Gold_Storage3.png"
#define GOLD_STORAGE_INFO_LV1    "buildings/Gold_Storage_Info1.png"
#define GOLD_STORAGE_INFO_LV2    "buildings/Gold_Storage_Info2.png"
#define GOLD_STORAGE_INFO_LV3    "buildings/Gold_Storage_Info3.png"
#define INFO_IMAGE_SCALE         0.8f

// ------------------------ 储金罐子类实现（1:1复刻大本营写法） ------------------------
GoldStorage* GoldStorage::create(int level) {
    GoldStorage* goldStorage = new (std::nothrow) GoldStorage();
    if (goldStorage && goldStorage->init(BuildingType::GOLD_STORAGE, level)) {
        goldStorage->autorelease();
        CCLOG("【储金罐】创建成功，初始等级：%d | 网格尺寸：%.0fx%.0f", level,
            GOLD_STORAGE_GRID_SIZE.width, GOLD_STORAGE_GRID_SIZE.height);
        return goldStorage;
    }
    CC_SAFE_DELETE(goldStorage);
    return nullptr;
}

bool GoldStorage::loadBuildingRes() {
    return loadGoldStorageResByLevel(_level);
}

bool GoldStorage::loadGoldStorageResByLevel(int level) {
    std::string resPath;
    switch (level) {
    case 1: resPath = GOLD_STORAGE_LEVEL_1_RES; break;
    case 2: resPath = GOLD_STORAGE_LEVEL_2_RES; break;
    case 3: resPath = GOLD_STORAGE_LEVEL_3_RES; break;
    default: resPath = GOLD_STORAGE_LEVEL_1_RES; break;
    }

    if (!Sprite::initWithFile(resPath)) {
        CCLOG("【储金罐】加载建筑资源失败：%s", resPath.c_str());
        return false;
    }

    _buildingSize = GOLD_STORAGE_SIZE;
    this->setContentSize(_buildingSize);
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->getTexture()->setAntiAliasTexParameters();

    // 适配菜单/信息图偏移（和大本营交互逻辑一致，适配3x3网格）
    m_menuOffset = Vec2(60, GOLD_STORAGE_SIZE.height / 2 - 105);
    m_infoImageOffset = Vec2(0, 45);

    return true;
}

bool GoldStorage::loadBuildingInfoRes() {
    return loadGoldStorageInfoResByLevel(_level);
}

bool GoldStorage::loadGoldStorageInfoResByLevel(int level) {
    // 销毁旧信息图，避免内存泄漏（复刻大本营逻辑）
    if (_infoImage) {
        _infoImage->removeFromParentAndCleanup(true);
        _infoImage = nullptr;
    }

    std::string resPath;
    switch (level) {
    case 1: resPath = GOLD_STORAGE_INFO_LV1; break;
    case 2: resPath = GOLD_STORAGE_INFO_LV2; break;
    case 3: resPath = GOLD_STORAGE_INFO_LV3; break;
    default: resPath = GOLD_STORAGE_INFO_LV1; break;
    }

    // 资源缺失时红色色块兜底（和大本营/金矿一致）
    _infoImage = Sprite::create(resPath);
    if (!_infoImage) {
        CCLOG("【储金罐】等级%d信息图加载失败，使用红色调试块", level);
        _infoImage = Sprite::create();
        _infoImage->setColor(Color3B::RED);
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

//  核心升级逻辑（复刻大本营 + 严格匹配圣水分级扣费要求）
bool GoldStorage::upgradeBuilding() {
    // 1. 最高等级校验（保留3级上限，和大本营/金矿一致）
    if (_level >= 3) {
        CCLOG("【储金罐】已达最高等级3，无法升级");
        return false;
    }

    //  2. 圣水分级扣费配置 + 圣水校验（核心逻辑，严格匹配要求）
    int costElixir = 0;
    GameManager* gm = GameManager::getInstance();
    if (!gm) {
        CCLOG("【储金罐】升级失败：游戏管理器实例为空");
        return false;
    }

    //  严格匹配要求：1→2耗750圣水、2→3耗1500圣水
    switch (_level) {
    case 1: costElixir = 750;  break;  // Lv1升Lv2，消耗750圣水
    case 2: costElixir = 1500; break;  // Lv2升Lv3，消耗1500圣水
    default: return false;
    }

    //  圣水不足：静默禁止升级，无弹窗、无提示（完全符合工程规范）
    if (!gm->spendResource(ResourceType::ELIXIR, costElixir)) {
        CCLOG("【储金罐】升级失败：圣水不足！当前%d，所需%d", gm->getResource(ResourceType::ELIXIR), costElixir);
        return false;
    }

    SceneMap::getInstance()->refreshResourceImmediately();

    // 3. 圣水扣除成功，执行升级（复刻大本营后续逻辑）
    int oldLevel = _level;
    _level++;

    // 4. 加载对应等级资源+信息图
    bool buildingUpgradeSuccess = loadGoldStorageResByLevel(_level);
    bool infoUpgradeSuccess = loadGoldStorageInfoResByLevel(_level);

    if (buildingUpgradeSuccess) {
        CCLOG("【储金罐】升级成功：Lv%d → Lv%d | 扣除圣水%d，剩余%d",
            oldLevel, _level, costElixir, gm->getResource(ResourceType::ELIXIR));

        // 升级后自动关闭菜单+隐藏信息图（和大本营交互一致）
        this->hideBuildingInfo();
        this->closeUpgradeMenu();
        loadBuildingInfoRes();

        if (!infoUpgradeSuccess) {
            CCLOG("【储金罐】建筑升级成功，但信息图更新失败");
            //  新增：刷新金币总上限（金矿升级，容量上涨）
            SceneMap::getInstance()->calculateTotalGoldCapacity();
        }
        return true;
    }
    else {
        //  升级失败：等级回滚 + 返还圣水（兜底逻辑，避免数据异常）
        _level = oldLevel;
        gm->addResource(ResourceType::ELIXIR, costElixir);
        CCLOG("【储金罐】升级失败，等级回滚至Lv%d，返还圣水%d", oldLevel, costElixir);
        return false;
    }
}

// ------------------------ 全局创建储金罐入口（和大本营格式一致） ------------------------
void createLevel1GoldStorage() {
    HomeVillageMap* homeMap = HomeVillageMap::getInstance("HomeVillageMap");
    if (!homeMap) {
        CCLOG("【创建储金罐】地图实例为空");
        return;
    }

    if (homeMap->getParent() == nullptr) {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
            createLevel1GoldStorage();
            });
        return;
    }

    GoldStorage* goldStorage = GoldStorage::create(1);
    if (!goldStorage) {
        CCLOG("【创建储金罐】创建实例失败");
        return;
    }

    // 储金罐默认生成位置（可根据你的地图调整坐标）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 spawnPos = Vec2(visibleSize.width * 0.8f, visibleSize.height * 0.3f);
    goldStorage->setBuildingPosition(spawnPos);

    homeMap->addChild(goldStorage, 200);
    CCLOG("【创建储金罐】1级储金罐创建成功，位置：(%.0f, %.0f) | 3x3网格适配完成",
        spawnPos.x, spawnPos.y);
}