#pragma once
// BuildingData.h
#ifndef __BUILDING_DATA_H__
#define __BUILDING_DATA_H__

#include <string>
#include <vector>
#include <map>

// 建筑类型枚举
enum class BuildingType {
    GOLD_MINE,          // 金矿
    ELIXIR_COLLECTOR,   // 圣水收集器
    ARCHER_TOWER,       // 弓箭塔
    CANNON,            // 加农炮
    ARMY_CAMP,         // 兵营
    TOWN_HALL,          // 大本营
    BARRACKS,            // 兵营
    GOLD_STORAGE,         // 储金罐
    ELIXIR_STORAGE       //圣水瓶
};

// 单等级建筑数据
struct BuildingLevelStats {
    int level;              // 等级
    int maxHp;              // 生命值
    int buildTime;          // 建造时间(秒)
    int goldCost;           // 升级金币消耗
    int elixirCost;         // 升级圣水消耗
    int damage;             // 伤害值(防御建筑)
    float attackSpeed;      // 攻击速度
    float attackRange;      // 攻击范围
    int productionRate;     // 生产速率(资源建筑)
    int storageCapacity;    // 存储容量
    std::string spriteName; // 使用的精灵图片名
};

// 整个建筑的数据（包含所有等级）
struct BuildingData {
    BuildingType type;      // 建筑类型
    std::string name;       // 建筑名称
    std::string description;// 描述
    int gridWidth;          // 占地宽(格子)
    int gridHeight;         // 占地高(格子)
    std::vector<BuildingLevelStats> levels; // 所有等级数据
};

// 建筑数据管理器
class BuildingConfig {
public:
    // 获取特定类型和等级的建筑数据
    static const BuildingLevelStats* getStats(BuildingType type, int level);

    // 获取建筑基础信息
    static const BuildingData* getBuildingData(BuildingType type);

    // 获取建筑最大等级
    static int getMaxLevel(BuildingType type);

    // 获取所有建筑类型（用于UI显示等）
    static std::vector<BuildingType> getAllBuildingTypes();

private:
    // 私有构造函数，禁止实例化
    BuildingConfig() = delete;

    // 静态初始化所有建筑数据
    static void initData();

    // 存储所有建筑数据的静态映射
    static std::map<BuildingType, BuildingData> s_buildingData;

    // 静态初始化器（C++11保证线程安全）
    static class StaticInitializer {
    public:
        StaticInitializer() { BuildingConfig::initData(); }
    } s_initializer;
};

#endif // __BUILDING_DATA_H__