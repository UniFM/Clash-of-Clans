// BuildingData.cpp
#include "BuildingData.h"
#include "Constant/Constant.h"

// 静态成员变量
std::map<BuildingType, BuildingData> BuildingConfig::s_buildingData;
BuildingConfig::StaticInitializer BuildingConfig::s_initializer;

// 初始化所有建筑数据
void BuildingConfig::initData() {
    // ========== 金矿数据 ==========
    BuildingData goldMine;
    goldMine.type = BuildingType::GOLD_MINE;
    goldMine.name = "金矿";
    goldMine.description = "生产金币的建筑";
    goldMine.gridWidth = 3;
    goldMine.gridHeight = 3;

    // 金矿等级1
    BuildingLevelStats goldLv1;
    goldLv1.level = 1;
    goldLv1.maxHp = 400;
    goldLv1.buildTime = 60;
    goldLv1.goldCost = 150;
    goldLv1.elixirCost = 0;
    goldLv1.productionRate = 200;
    goldLv1.storageCapacity = 1000;
    goldLv1.spriteName = "gold_mine_lv1.png";
    goldMine.levels.push_back(goldLv1);

    // 金矿等级2
    BuildingLevelStats goldLv2;
    goldLv2.level = 2;
    goldLv2.maxHp = 600;
    goldLv2.buildTime = 300;
    goldLv2.goldCost = 500;
    goldLv2.elixirCost = 0;
    goldLv2.productionRate = 400;
    goldLv2.storageCapacity = 2000;
    goldLv2.spriteName = "gold_mine_lv2.png";
    goldMine.levels.push_back(goldLv2);

    s_buildingData[BuildingType::GOLD_MINE] = goldMine;

    // ========== 箭塔数据 ==========
    BuildingData archerTower;
    archerTower.type = BuildingType::ARCHER_TOWER;
    archerTower.name = "箭塔";
    archerTower.description = "对空对地防御建筑";
    archerTower.gridWidth = 3;
    archerTower.gridHeight = 3;

    BuildingLevelStats archerLv1;
    archerLv1.level = 1;
    archerLv1.maxHp = 800;
    archerLv1.buildTime = 300;
    archerLv1.goldCost = 1000;
    archerLv1.elixirCost = 0;
    archerLv1.damage = 25;
    archerLv1.attackSpeed = 0.8f;
    archerLv1.attackRange = 300.0f;
    archerLv1.spriteName = "archer_tower_lv1.png";
    archerTower.levels.push_back(archerLv1);

    s_buildingData[BuildingType::ARCHER_TOWER] = archerTower;

    // ========== 加农炮数据 ==========
    BuildingData cannon;
    cannon.type = BuildingType::CANNON;
    cannon.name = "cannon";
    cannon.description = "对地防御建筑";
    cannon.gridWidth = 3;
    cannon.gridHeight = 3;

    BuildingLevelStats cannonLv1;
    cannonLv1.level = 1;
    cannonLv1.maxHp = 300;          // 图中加农炮1级生命值
    cannonLv1.buildTime = 5;        // 图中加农炮1级建造/升级时间（秒）
    cannonLv1.goldCost = 250;       // 图中加农炮1级金叶花费
    cannonLv1.elixirCost = 0;
    cannonLv1.damage = 5.6;         // 图中加农炮1级每秒伤害
    cannonLv1.attackSpeed = 0.8f;   // 攻击速度，逻辑上5.6/7=0.8（大概）
    cannonLv1.attackRange = 300.0f; // 假设显示圆中的半径数值，图中未提供则保持一致
    cannonLv1.spriteName = ResPath::CANNONLEVEL1;  // 加农炮1级图片图标

    cannon.levels.push_back(cannonLv1);

    s_buildingData[BuildingType::CANNON] = cannon;

    // ========== 兵营数据 ==========
    BuildingData armyCamp;
    armyCamp.type = BuildingType::ARMY_CAMP;
    armyCamp.name = "兵营";
    armyCamp.description = "训练士兵的建筑";
    armyCamp.gridWidth = 4;
    armyCamp.gridHeight = 4;

    BuildingLevelStats campLv1;
    campLv1.level = 1;
    campLv1.maxHp = 700;
    campLv1.buildTime = 180;
    campLv1.goldCost = 500;
    campLv1.elixirCost = 0;
    campLv1.storageCapacity = 20; // 初始容量20人口
    campLv1.spriteName = "army_camp_lv1.png";
    armyCamp.levels.push_back(campLv1);

    s_buildingData[BuildingType::ARMY_CAMP] = armyCamp;

    // ========== 圣水收集器 ==========
    BuildingData elixirCollector;
    elixirCollector.type = BuildingType::ELIXIR_COLLECTOR;
    elixirCollector.name = "圣水收集器";
    elixirCollector.description = "采集圣水的建筑";
    elixirCollector.gridWidth = 3;
    elixirCollector.gridHeight = 3;

    BuildingLevelStats elixirLv1;
    elixirLv1.level = 1;
    elixirLv1.maxHp = 400;
    elixirLv1.buildTime = 60;
    elixirLv1.goldCost = 150;
    elixirLv1.elixirCost = 0;
    elixirLv1.productionRate = 200;
    elixirLv1.storageCapacity = 1000;
    elixirLv1.spriteName = "elixir_collector_lv1.png";
    elixirCollector.levels.push_back(elixirLv1);

    s_buildingData[BuildingType::ELIXIR_COLLECTOR] = elixirCollector;

    // ========== 大本营 ==========
    BuildingData townHall;
    townHall.type = BuildingType::TOWN_HALL;
    townHall.name = "大本营";
    townHall.description = "村庄的核心";
    townHall.gridWidth = 4;
    townHall.gridHeight = 4;

    BuildingLevelStats thLv1;
    thLv1.level = 1;
    thLv1.maxHp = 1500;
    thLv1.buildTime = 0;
    thLv1.goldCost = 0;
    thLv1.elixirCost = 0;
    thLv1.storageCapacity = 1000;
    thLv1.spriteName = ResPath::TOWNHALLLEVEL1;
    townHall.levels.push_back(thLv1);

    s_buildingData[BuildingType::TOWN_HALL] = townHall;

    // ========== 兵营 (Barracks) - 造兵用 ==========
    BuildingData barracks;
    barracks.type = BuildingType::BARRACKS;
    barracks.name = "训练营";
    barracks.description = "训练军队";
    barracks.gridWidth = 3;
    barracks.gridHeight = 3;

    BuildingLevelStats barracksLv1;
    barracksLv1.level = 1;
    barracksLv1.maxHp = 450;
    barracksLv1.buildTime = 60;
    barracksLv1.goldCost = 200;
    barracksLv1.elixirCost = 0;
    barracksLv1.spriteName = "barracks_lv1.png";
    barracks.levels.push_back(barracksLv1);

    s_buildingData[BuildingType::BARRACKS] = barracks;

    // ========== 储金罐 ==========
    BuildingData goldStorage;
    goldStorage.type = BuildingType::GOLD_STORAGE;
    goldStorage.name = "储金罐";
    goldStorage.description = "储存金币";
    goldStorage.gridWidth = 3;
    goldStorage.gridHeight = 3;

    BuildingLevelStats goldStorageLv1;
    goldStorageLv1.level = 1;
    goldStorageLv1.maxHp = 800;
    goldStorageLv1.buildTime = 300;
    goldStorageLv1.goldCost = 300;
    goldStorageLv1.elixirCost = 0;
    goldStorageLv1.storageCapacity = 1500;
    goldStorageLv1.spriteName = "gold_storage_lv1.png";
    goldStorage.levels.push_back(goldStorageLv1);

    s_buildingData[BuildingType::GOLD_STORAGE] = goldStorage;

    // ========== 圣水瓶 ==========
    BuildingData elixirStorage;
    elixirStorage.type = BuildingType::ELIXIR_STORAGE;
    elixirStorage.name = "圣水瓶";
    elixirStorage.description = "储存圣水";
    elixirStorage.gridWidth = 3;
    elixirStorage.gridHeight = 3;

    BuildingLevelStats elixirStorageLv1;
    elixirStorageLv1.level = 1;
    elixirStorageLv1.maxHp = 800;
    elixirStorageLv1.buildTime = 300;
    elixirStorageLv1.goldCost = 300;
    elixirStorageLv1.elixirCost = 0;
    elixirStorageLv1.storageCapacity = 1500;
    elixirStorageLv1.spriteName = "elixir_storage_lv1.png";
    elixirStorage.levels.push_back(elixirStorageLv1);

    s_buildingData[BuildingType::ELIXIR_STORAGE] = elixirStorage;

    CCLOG("Building data initialized: %zu types loaded", s_buildingData.size());
};

// 获取特定类型和等级的建筑统计数据
const BuildingLevelStats* BuildingConfig::getStats(BuildingType type, int level) {
    auto it = s_buildingData.find(type);
    if (it == s_buildingData.end() || level < 1 || level > it->second.levels.size()) {
        return nullptr;
    }
    return &(it->second.levels[level - 1]);
}

// 获取建筑基础信息
const BuildingData* BuildingConfig::getBuildingData(BuildingType type) {
    auto it = s_buildingData.find(type);
    return (it != s_buildingData.end()) ? &(it->second) : nullptr;
}

// 获取最大等级
int BuildingConfig::getMaxLevel(BuildingType type) {
    auto it = s_buildingData.find(type);
    return (it != s_buildingData.end()) ? it->second.levels.size() : 0;
}

// 获取所有建筑类型
std::vector<BuildingType> BuildingConfig::getAllBuildingTypes() {
    std::vector<BuildingType> types;
    for (const auto& pair : s_buildingData) {
        types.push_back(pair.first);
    }
    return types;
}
