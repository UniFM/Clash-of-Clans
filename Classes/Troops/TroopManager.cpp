/*************************************************************
* @file     : TroopManager.cpp
* @function ：兵种管理器实现 - 单例管控兵种数量与人口容量
* @author   : 俞筱雨
* @note     : 1. 单例模式管理兵种选中数量、兵营等级配置；
*             2. 计算兵营人口容量，判断兵种添加容量是否充足；
*             3. 封装兵种数量增删、清空及容量占用计算逻辑。
**************************************************************/


#include "TroopManager.h"

// 单例对象初始化（空指针）
TroopManager* TroopManager::instance = nullptr;

// 获取兵种管理器单例实例
TroopManager* TroopManager::getInstance() {
    if (!instance) {
        instance = new TroopManager();
    }
    return instance;
}

// 构造函数：初始化兵营等级为1，所有兵种数量为0
TroopManager::TroopManager() : barracksLevel(1) {
    // 初始化各兵种选中数量为0
    selectedTroops[TroopType::BARBARIAN] = 0;
    selectedTroops[TroopType::ARCHER] = 0;
    selectedTroops[TroopType::GOBLIN] = 0;
    selectedTroops[TroopType::GIANT] = 0;
}

// 设置兵营等级
void TroopManager::setBarracksLevel(int level) {
    barracksLevel = level;
}

// 获取兵营当前等级
int TroopManager::getBarracksLevel() const {
    Barracks* barracks = Barracks::create(1); // 创建兵营实例
    int currentLevel = barracks->getLevel();   // 获取兵营等级
    CCLOG("兵营当前等级：%d", currentLevel);
    return currentLevel;
}

// 计算最大人口容量：等级1=20，等级2=30，以此类推
int TroopManager::getMaxHousingSpace() const {
    return 10 + (barracksLevel * 10);
}

// 设置指定兵种的选中数量（数量不能为负）
void TroopManager::setTroopCount(TroopType type, int count) {
    if (count < 0) return;
    selectedTroops[type] = count;
}

// 获取指定兵种的选中数量
int TroopManager::getTroopCount(TroopType type) const {
    auto it = selectedTroops.find(type);
    if (it != selectedTroops.end()) {
        return it->second;
    }
    return 0;
}

// 清空所有兵种的选中数量（重置为0）
void TroopManager::clearTroops() {
    for (auto& pair : selectedTroops) {
        pair.second = 0;
    }
}

// 计算当前已占用的总人口容量
int TroopManager::getCurrentHousingSpace() const {
    int total = 0;
    for (const auto& pair : selectedTroops) {
        // 总容量 = 兵种数量 * 该兵种单单位占用容量
        total += pair.second * TROOP_DATA.at(pair.first).housingSpace;
    }
    return total;
}

// 判断是否可添加指定兵种（剩余容量是否足够）
bool TroopManager::canAddTroop(TroopType type) const {
    int spaceNeeded = TROOP_DATA.at(type).housingSpace;
    // 当前占用容量 + 新增兵种所需容量 <= 最大容量
    return (getCurrentHousingSpace() + spaceNeeded) <= getMaxHousingSpace();
}