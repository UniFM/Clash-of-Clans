// BuildingData.cpp
#include "BuildingData.h"
#include "Constant/Constant.h"

// 静态成员定义
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

    // ========== 弓箭塔数据 ==========
    BuildingData archerTower;
    archerTower.type = BuildingType::ARCHER_TOWER;
    archerTower.name = "弓箭塔";
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
    cannon.description = "对空对地防御建筑";
    cannon.gridWidth = 3;
    cannon.gridHeight = 3;

    BuildingLevelStats cannonLv1;
    cannonLv1.level = 1;
    cannonLv1.maxHp = 300;          // 图中加农炮1级生命值
    cannonLv1.buildTime = 5;        // 图中加农炮1级建造/升级时间（秒）
    cannonLv1.goldCost = 250;       // 图中加农炮1级金币费用
    cannonLv1.elixirCost = 0;
    cannonLv1.damage = 5.6;         // 图中加农炮1级每次伤害
    cannonLv1.attackSpeed = 0.8f;   // 按攻击间隔逻辑（5.6/7=0.8）填充
    cannonLv1.attackRange = 300.0f; // 沿用示例中的射程数值（图中未提供则保持一致）
    cannonLv1.spriteName = ResPath::CANNONLEVEL1;  // 加农炮1级精灵图名

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
    campLv1.storageCapacity = 20; // 可容纳20人口
    campLv1.spriteName = "army_camp_lv1.png";
    armyCamp.levels.push_back(campLv1);

    s_buildingData[BuildingType::ARMY_CAMP] = armyCamp;

    CCLOG("建筑数据初始化完成！共%f种建筑。", s_buildingData.size());
}

// 获取建筑统计数据
const BuildingLevelStats* BuildingConfig::getStats(BuildingType type, int level) {
    auto it = s_buildingData.find(type);
    if (it == s_buildingData.end() || level < 1 || level > it->second.levels.size()) {
        return nullptr;
    }
    return &(it->second.levels[level - 1]);
}

// 获取建筑基础数据
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